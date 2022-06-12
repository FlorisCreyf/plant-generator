/* Copyright 2022 Floris Creyf
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "generator.h"
#include "util.h"

using namespace pg;
using std::pair;
using std::vector;
typedef Mesh::Segment Segment;
typedef Mesh::State State;

const float pi = 3.14159265359f;

MeshGenerator::MeshGenerator(Plant *plant) :
	plant(plant),
	collarGenerator(plant, mesh),
	forkGenerator(mesh),
	mesh(plant)
{

}

const Mesh &MeshGenerator::getMesh()
{
	return this->mesh;
}

const Mesh &MeshGenerator::generate()
{
	Stem *stem = this->plant->getRoot();
	this->mesh.initBuffer();
	if (stem) {
		State parentState = {};
		State state;
		state.prevRotation = Quat(0.0f, 0.0f, 0.0f, 1.0f);
		state.prevDirection = Vec3(0.0f, 0.0f, 1.0f);
		addStem(stem, state, parentState, false);
		this->mesh.updateSegments();
	}
	return this->mesh;
}

size_t getSectionCount(Stem *stem, Stem *fork)
{
	const Path &path = stem->getPath();
	if (fork)
		return path.getSize() - path.getDivisions() - 1;
	else
		return path.getSize();
}

void MeshGenerator::addSections(
	State &state, Segment parent, bool isFork, Stem *fork)
{
	Stem *stem = state.segment.stem;
	state.prevIndex = this->mesh.vertices[state.mesh].size();
	if (stem->getSectionDivisions() != this->mesh.section.getResolution())
		this->mesh.section.generate(stem->getSectionDivisions());

	if (isFork)
		addForkSection(stem, state);
	else {
		state.section = 0;
		state.texOffset = 0.0f;
		Vec2 swelling = stem->getSwelling();
		if (swelling.x >= 1.0f && swelling.y >= 1.0f) {
			createBranchCollar(state, parent);
			this->collarGenerator.connectCollar(state, fork);
		}
	}

	size_t sections = getSectionCount(stem, fork);
	for (; state.section < sections; state.section++) {
		Quat rotation = rotateSection(state);
		state.prevIndex = this->mesh.vertices[state.mesh].size();
		addSection(state, rotation, this->mesh.section);
		if (state.section+1 < sections)
			this->mesh.addTriangleRing(
				state.prevIndex,
				this->mesh.vertices[state.mesh].size(),
				stem->getSectionDivisions(),
				state.mesh);
	}

	if (fork) {
		addForkBaseSection(fork, stem, state);
	} else if (stem->getMinRadius() > 0.0f)
		capStem(stem, state.mesh, state.prevIndex);
}

/** Generate a cross section for a point in the stem's path. Indices are added
at a later stage to connect the sections. */
void MeshGenerator::addSection(
	State &state, Quat rotation, const CrossSection &section)
{
	Stem *stem = state.segment.stem;
	size_t index = state.section;
	DVertex vertex;
	vertex.tangentScale = 1.0f;
	vertex.tangent = stem->getPath().getAverageDirection(index);
	vertex.uv.y = getTextureLength(stem, index) + state.texOffset;
	state.texOffset = vertex.uv.y;
	Vec3 location = stem->getLocation();
	location += stem->getPath().get(index);

	Vec2 indices;
	Vec2 weights;
	if (stem->getJoints().size() > 0)
		updateJointState(state, indices, weights);
	else {
		indices = Vec2(state.jointID, 0.0f);
		weights = Vec2(1.0f, 0.0f);
	}

	float radius = this->plant->getRadius(stem, index);
	const std::vector<SVertex> sectionVertices = section.getVertices();
	for (size_t i = 0; i < sectionVertices.size(); i++) {
		vertex.position = sectionVertices[i].position;
		vertex.position = radius * vertex.position;
		vertex.position = rotate(rotation, vertex.position);
		vertex.position += location;
		vertex.normal = sectionVertices[i].normal;
		vertex.normal = rotate(rotation, vertex.normal);
		vertex.normal = normalize(vertex.normal);
		vertex.uv.x = sectionVertices[i].uv.x;
		vertex.weights = weights;
		vertex.indices = indices;
		this->mesh.vertices[state.mesh].push_back(vertex);
	}
}

Segment MeshGenerator::addStem(
	Stem *stem, State &state, State parentState, bool isFork)
{
	Stem *fork[2];
	stem->getFork(fork);
	if (!this->forkGenerator.isValidFork(stem, fork))
		fork[0] = fork[1] = nullptr;

	state.mesh = stem->getMaterial(Stem::Outer);
	state.segment.stem = stem;
	state.segment.vertexStart = this->mesh.vertices[state.mesh].size();
	state.segment.indexStart = this->mesh.indices[state.mesh].size();
	setInitialJointState(state, parentState);
	addSections(state, parentState.segment, isFork, fork[0]);
	state.segment.vertexCount = this->mesh.vertices[state.mesh].size();
	state.segment.vertexCount -= state.segment.vertexStart;
	state.segment.indexCount = this->mesh.indices[state.mesh].size();
	state.segment.indexCount -= state.segment.indexStart;
	addLeaves(stem, state);

	if (fork[0])
		createFork(fork, state);
	if (!isFork)
		/* The parent stem finishes generating both forks and will
		generate child stems afterwards. */
		addChildStems(stem, fork, state);

	return state.segment;
}

void MeshGenerator::addChildStems(Stem *stem, Stem *fork[2], State &state)
{
	this->mesh.stems[state.mesh].emplace(stem, state.segment);
	Stem *child = stem->getChild();
	while (child) {
		if (fork[0] != child && fork[1] != child) {
			State childState;
			setInitialRotation(child, childState);
			addStem(child, childState, state, false);
		}
		child = child->getSibling();
	}
}

/** Create two cross sections and connect them with Bezier curves. */
void MeshGenerator::createBranchCollar(State &state, Segment parentSegment)
{
	Stem *stem = state.segment.stem;
	State originalState = state;
	addSection(state, rotateSection(state), this->mesh.section);
	size_t start = this->mesh.vertices[state.mesh].size();
	this->collarGenerator.reserveBranchCollarSpace(stem, state.mesh);
	state.prevIndex = this->mesh.vertices[state.mesh].size();
	state.texOffset = 0.0f;
	state.section = stem->getPath().getInitialDivisions() + 1;
	state.prevIndex = this->mesh.vertices[state.mesh].size();
	addSection(state, rotateSection(state), this->mesh.section);
	state.section = this->collarGenerator.insertCollar(
		state.segment, parentSegment, start);
	if (state.section == 0)
		state = originalState;
}

void MeshGenerator::addForkBaseSection(Stem *fork, Stem *stem, State &state)
{
	if (fork->getPath().getInitialDivisions() == 0)
		this->mesh.addTriangleRing(
			state.prevIndex,
			this->mesh.vertices[state.mesh].size(),
			stem->getSectionDivisions(),
			state.mesh);
	else
		this->forkGenerator.reserveForkSpace(fork, state.mesh);

	Quat rotation = rotateSection(state);
	state.prevIndex = this->mesh.vertices[state.mesh].size();
	size_t section1 = state.section;
	size_t section2 = stem->getPath().getSize() - 1;
	state.texOffset += getTextureLength(stem, section1, section2);
	state.section = section2;
	addSection(state, rotation, this->mesh.section);
}

void MeshGenerator::addForkSection(Stem *stem, State &state)
{
	Quat rotation = state.prevRotation;
	size_t section = state.section;
	state.section = 0;
	addSection(state, rotation, this->mesh.section);
	state.section = section;
	state.texOffset += getTextureLength(stem, 0, section - 1);

	if (stem->getPath().getInitialDivisions() == 0)
		this->mesh.addTriangleRing(
			state.prevIndex,
			this->mesh.vertices[state.mesh].size(),
			stem->getSectionDivisions(),
			state.mesh);
	else
		this->forkGenerator.reserveForkSpace(stem, state.mesh);
}

void MeshGenerator::createFork(Stem *stems[2], State &state)
{
	const int cd = stems[0]->getPath().getInitialDivisions();
	const Vec3 direction1 = normalize(stems[0]->getPath().get(cd+1));
	const Vec3 direction2 = normalize(stems[1]->getPath().get(cd+1));
	Fork::Section middle = this->forkGenerator.getMiddle(stems, state);
	this->forkGenerator.reserveExtraTriangles(middle, state);

	Segment segments[2];
	State fs[2];
	fs[0].mesh = stems[0]->getMaterial(Stem::Outer);
	fs[0].section = cd + 1;
	fs[0].texOffset = state.texOffset;
	fs[0].prevRotation = rotateIntoVecQ(state.prevDirection, direction1);
	fs[0].prevRotation *= state.prevRotation;
	fs[0].prevDirection = direction1;
	segments[0] = addStem(stems[0], fs[0], state, true);
	fs[1].mesh = stems[1]->getMaterial(Stem::Outer);
	fs[1].section = cd + 1;
	fs[1].texOffset = state.texOffset;
	fs[1].prevRotation = rotateIntoVecQ(state.prevDirection, direction2);
	fs[1].prevRotation *= state.prevRotation;
	fs[1].prevDirection = direction2;
	segments[1] = addStem(stems[1], fs[1], state, true);

	this->mesh.stems[state.mesh].emplace(state.segment.stem, state.segment);
	this->forkGenerator.connectForks(stems, segments, fs, state, middle);

	for (int i = 0; i < 2; i++) {
		Stem *childFork[2];
		stems[i]->getFork(childFork);
		if (!this->forkGenerator.isValidFork(stems[i], childFork))
			childFork[0] = childFork[1] = nullptr;
		addChildStems(stems[i], childFork, fs[i]);
	}
}

/** The cross section is rotated so that the first point is always the topmost
point relative to the parent stem direction. */
void MeshGenerator::setInitialRotation(Stem *stem, State &state)
{
	float position = stem->getDistance();
	const Path &parentPath = stem->getParent()->getPath();
	Vec3 parentDirection = parentPath.getIntermediateDirection(position);
	const Path &path = stem->getPath();
	Vec3 stemDirection = path.getDirection(0);

	Vec3 up(0.0f, 0.0f, 1.0f);
	state.prevRotation = rotateIntoVecQ(up, stemDirection);
	state.prevDirection = stemDirection;

	Vec3 sideways(1.0f, 0.0f, 0.0f);
	sideways = normalize(rotate(state.prevRotation, sideways));
	up = projectOntoPlane(parentDirection, stemDirection);
	up = normalize(dot(up, sideways) * up);

	Quat rotation = rotateIntoVecQ(sideways, up);
	state.prevRotation = rotation * state.prevRotation;
}

Quat MeshGenerator::rotateSection(State &state)
{
	Stem *stem = state.segment.stem;
	const Path &path = stem->getPath();
	Vec3 direction = path.getAverageDirection(state.section);
	Quat rotation = rotateIntoVecQ(state.prevDirection, direction);
	rotation *= state.prevRotation;
	state.prevRotation = rotation;
	state.prevDirection = direction;
	return rotation;
}

float MeshGenerator::getTextureLength(Stem *stem, size_t section)
{
	if (section > 0) {
		float length = stem->getPath().getSegmentLength(section);
		float radius = this->plant->getRadius(stem, section - 1);
		float aspect = getAspect(this->plant, stem);
		return (length * aspect) / (radius * 2.0f * pi);
	} else
		return 0.0f;
}

float MeshGenerator::getTextureLength(
	Stem *stem, size_t section1, size_t section2)
{
	Vec3 p1 = stem->getPath().get(section1);
	Vec3 p2 = stem->getPath().get(section2);
	float length = magnitude(p2 - p1);
	float radius = this->plant->getRadius(stem, section1);
	float aspect = getAspect(this->plant, stem);
	return (length * aspect) / (radius * 2.0f * pi);
}

void MeshGenerator::capStem(Stem *stem, int stemMesh, size_t section)
{
	long mesh = stem->getMaterial(Stem::Inner);
	size_t index = section;
	size_t divisions = stem->getSectionDivisions();
	float rotation = 2.0f * pi / divisions;
	float angle = 0.0f;
	section = this->mesh.vertices[mesh].size();

	for (size_t i = 0; i <= divisions; i++, index++, angle += rotation) {
		DVertex vertex = this->mesh.vertices[stemMesh][index];
		vertex.uv.x = std::cos(angle) * 0.5f + 0.5f;
		vertex.uv.y = std::sin(angle) * 0.5f + 0.5f;
		this->mesh.vertices[mesh].push_back(vertex);
	}

	for (index = 0; index < divisions/2 - 1; index++) {
		this->mesh.addTriangle(
			mesh,
			section + index,
			section + divisions - index - 1,
			section + index + 1);
		this->mesh.addTriangle(
			mesh,
			section + index + 1,
			section + divisions - index - 1,
			section + divisions - index - 2);
	}

	if ((divisions & 1) != 0) {
		size_t lastSection = section + index;
		this->mesh.addTriangle(
			mesh, lastSection, lastSection+2, lastSection+1);
	}
}

void MeshGenerator::addLeaves(Stem *stem, const State &state)
{
	auto leaves = stem->getLeaves();
	unsigned index = 0;
	for (auto it = leaves.begin(); it != leaves.end(); it++)
		addLeaf(stem, index++, state);
}

void MeshGenerator::addLeaf(Stem *stem, unsigned leafIndex, const State &state)
{
	Leaf *leaf = stem->getLeaf(leafIndex);
	long mesh = leaf->getMaterial();
	Segment leafSegment;
	leafSegment.leafIndex = leafIndex;
	leafSegment.stem = stem;
	leafSegment.vertexStart = this->mesh.vertices[mesh].size();
	leafSegment.indexStart = this->mesh.indices[mesh].size();

	Vec2 weights;
	Vec2 indices;
	if (stem->hasJoints()) {
		float position = leaf->getPosition();
		auto pair = getJoint(position, stem);
		size_t index = pair.second.getPathIndex();
		float jointPosition = stem->getPath().getDistance(index);
		float offset = position - jointPosition;
		setJointInfo(stem, offset, pair.first, weights, indices);
	} else {
		weights.x = 1.0f;
		weights.y = 0.0f;
		indices.x = static_cast<float>(state.jointID);
		indices.y = indices.x;
	}

	Geometry geom = transformLeaf(leaf, stem);
	size_t vsize = this->mesh.vertices[mesh].size();
	for (DVertex vertex : geom.getPoints()) {
		vertex.indices = indices;
		vertex.weights = weights;
		this->mesh.vertices[mesh].push_back(vertex);
	}
	for (unsigned i : geom.getIndices())
		this->mesh.indices[mesh].push_back(i + vsize);

	leafSegment.vertexCount = this->mesh.vertices[mesh].size();
	leafSegment.vertexCount -= leafSegment.vertexStart;
	leafSegment.indexCount = this->mesh.indices[mesh].size();
	leafSegment.indexCount -= leafSegment.indexStart;
	this->mesh.leaves[mesh].emplace(
		Mesh::LeafID(stem, leafIndex), leafSegment);
}

Geometry MeshGenerator::transformLeaf(const Leaf *leaf, const Stem *stem)
{
	const Path path = stem->getPath();
	Vec3 location = stem->getLocation();
	float position = leaf->getPosition();

	if (position >= 0.0f && position < path.getLength())
		location += path.getIntermediate(position);
	else
		location += path.get().back();

	Geometry geom = this->plant->getLeafMesh(leaf->getMesh());
	geom.transform(leaf->getRotation(), leaf->getScale(), location);
	return geom;
}

/** Stem descendants might not have joints and the parent state is needed to
determine what joint ancestors are influenced by. */
void MeshGenerator::setInitialJointState(State &state, const State &parentState)
{
	const Stem *stem = state.segment.stem;
	const Stem *parent = stem->getParent();
	state.jointID = 0;
	state.jointIndex = 0;
	state.jointOffset = 0.0f;
	const vector<Joint> joints = stem->getJoints();

	if (joints.empty() && (!parent || !parent->hasJoints())) {
		state.jointID = parentState.jointID;
	} else if (joints.empty()) {
		float position = stem->getDistance();
		auto pair = getJoint(position, stem->getParent());
		state.jointID = pair.second.getID();
		state.jointIndex = pair.first;
	} else {
		Joint joint = joints.front();
		state.jointID = joint.getID();
	}
}

pair<size_t, Joint> MeshGenerator::getJoint(float position, const Stem *stem)
{
	size_t index = stem->getPath().getIndex(position);
	const vector<Joint> joints = stem->getJoints();
	size_t jointIndex = 0;
	for (auto it = joints.begin(); it != joints.end(); it++) {
		size_t pathIndex = it->getPathIndex();
		if (pathIndex > index) {
			if (it != joints.begin()) {
				jointIndex--;
				it--;
			}
			return pair<size_t, Joint>(jointIndex, *it);
		}
		jointIndex++;
	}
	return pair<size_t, Joint>(jointIndex-1, joints.back());
}

void MeshGenerator::incrementJoint(State &state, const vector<Joint> &joints)
{
	if (state.jointIndex + 1 < joints.size()) {
		Joint nextJoint = joints[state.jointIndex + 1];
		if (nextJoint.getPathIndex() == state.section) {
			state.jointIndex++;
			state.jointID = nextJoint.getID();
			state.jointOffset = 0.0f;
		}
	}
}

/** Update the current joint and set the joint indices and weights. */
void MeshGenerator::updateJointState(State &state, Vec2 &indices, Vec2 &weights)
{
	const Stem *stem = state.segment.stem;
	const Path &path = stem->getPath();
	const vector<Joint> joints = stem->getJoints();
	incrementJoint(state, joints);
	size_t pathIndex = joints[state.jointIndex].getPathIndex();

	if (state.jointIndex == 0 && state.section <= pathIndex) {
		weights.x = 1.0f;
		weights.y = 0.0f;
		indices.x = static_cast<float>(state.jointID);
		indices.y = indices.x;
	} else if (state.section == 0 || state.section == path.getSize() - 1) {
		weights.x = 1.0f;
		weights.y = 0.0f;
		indices.x = static_cast<float>(state.jointID);
		indices.y = indices.x;
	} else if (state.section == pathIndex) {
		unsigned prevID = joints[state.jointIndex-1].getID();
		weights.x = 0.5f;
		weights.y = 0.5f;
		indices.x = static_cast<float>(state.jointID);
		indices.y = static_cast<float>(prevID);
	} else {
		Vec3 point1 = path.get(state.section);
		Vec3 point2 = path.get(state.section - 1);
		state.jointOffset += magnitude(point1 - point2);
		setJointInfo(stem, state.jointOffset, state.jointIndex,
			weights, indices);
	}
}

void MeshGenerator::setJointInfo(const Stem *stem, float jointOffset,
	size_t jointIndex, Vec2 &weights, Vec2 &indices)
{
	const vector<Joint> joints = stem->getJoints();
	const Path &path = stem->getPath();
	size_t pathIndex = joints[jointIndex].getPathIndex();
	unsigned jointID = joints[jointIndex].getID();

	float ratio;
	float distance;
	bool lastJoint = jointIndex + 1 >= joints.size();
	if (lastJoint) {
		size_t start = pathIndex;
		size_t end = path.getSize() - 1;
		distance = path.getDistance(start, end);
		ratio = jointOffset / distance;
	} else {
		Joint nextJoint = joints[jointIndex + 1];
		size_t start = pathIndex;
		size_t end = nextJoint.getPathIndex();
		distance = path.getDistance(start, end);
		ratio = jointOffset / distance;
	}

	bool first = ratio < 0.5f && jointIndex == 0;
	bool last = ratio > 0.5f && lastJoint;
	if (ratio == 0.5f || first || last) {
		weights.x = 1.0f;
		weights.y = 0.0f;
		indices.x = static_cast<float>(jointID);
		indices.y = indices.x;
	} else if (ratio > 0.5f) {
		int nextID = joints[jointIndex + 1].getID();
		indices.x = static_cast<float>(jointID);
		indices.y = static_cast<float>(nextID);
		float ratio = jointOffset / distance - 0.5f;
		weights.x = 1.0f - ratio;
		weights.y = ratio;
	} else {
		int prevID = joints[jointIndex - 1].getID();
		indices.x = static_cast<float>(jointID);
		indices.y = static_cast<float>(prevID);
		float ratio = jointOffset / distance;
		weights.x = 0.5f + ratio;
		weights.y = 0.5f - ratio;
	}
}

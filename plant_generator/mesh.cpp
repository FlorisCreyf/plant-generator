/* Copyright 2017 Floris Creyf
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

#include "mesh.h"
#include <cmath>
#include <limits>

#define PI 3.14159265359f

using namespace pg;
using std::map;
using std::pair;
using std::vector;

Mesh::Mesh(Plant *plant)
{
	this->plant = plant;
	this->defaultLeaf.setPlane();
}

void Mesh::generate()
{
	Stem *stem = this->plant->getRoot();
	initBuffer();
	if (stem) {
		State parentState = {};
		State state;
		state.prevRotation = Quat(0.0f, 0.0f, 0.0f, 1.0f);
		state.prevDirection = Vec3(0.0f, 1.0f, 0.0f);
		addStem(stem, state, parentState, false);
		updateSegments();
	}
}

bool isValidFork(Stem *stem, Stem *fork[2])
{
	if (fork[0]) {
		int d0 = stem->getSectionDivisions();
		int m1 = fork[0]->getMaterial(Stem::Outer);
		int m2 = fork[1]->getMaterial(Stem::Outer);
		int d1 = fork[0]->getSectionDivisions();
		int d2 = fork[1]->getSectionDivisions();
		if (d1 != d2 || d1 != d0 || d1 % 2 != 0 || m1 != m2) {
			fork[0] = nullptr;
			fork[1] = nullptr;
		}
	}
	return fork[0] && fork[1];
}

Segment Mesh::addStem(Stem *stem, State state, State parentState, bool isFork)
{
	Stem *fork[2];
	stem->getFork(fork);
	bool hasFork = isValidFork(stem, fork);

	state.mesh = stem->getMaterial(Stem::Outer);
	state.segment.stem = stem;
	state.segment.vertexStart = this->vertices[state.mesh].size();
	state.segment.indexStart = this->indices[state.mesh].size();
	setInitialJointState(state, parentState);
	addSections(state, parentState.segment, isFork, hasFork);
	state.segment.vertexCount = this->vertices[state.mesh].size();
	state.segment.vertexCount -= state.segment.vertexStart;
	state.segment.indexCount = this->indices[state.mesh].size();
	state.segment.indexCount -= state.segment.indexStart;
	this->stemSegments[state.mesh].emplace(stem, state.segment);
	addLeaves(stem, state);

	if (hasFork)
		if (!addForks(fork, state))
			fork[0] = fork[1] = nullptr;

	Stem *child = stem->getChild();
	while (child != nullptr) {
		if (fork[0] != child && fork[1] != child) {
			State childState;
			setInitialRotation(child, childState);
			addStem(child, childState, state, false);
		}
		child = child->getSibling();
	}

	return state.segment;
}

/** Remove cross sections from the end of a path until the distance is greater
than the radius. */
size_t getSectionCount(Stem *stem, bool hasFork)
{
	size_t sections = stem->getPath().getSize();
	if (hasFork) {
		Path path = stem->getPath();
		float radius = stem->getMinRadius();
		float t = 0.0f;
		for (sections--; sections > 0 && t <= radius; sections--) {
			Vec3 p1 = path.get(sections);
			Vec3 p2 = path.get(sections-1);
			t += pg::magnitude(p1-p2);
		}
		sections++;
	}
	return sections;
}

void Mesh::addSections(
	State &state, Segment parentSegment, bool isFork, bool hasFork)
{
	Stem *stem = state.segment.stem;
	state.prevIndex = this->vertices[state.mesh].size();
	if (stem->getSectionDivisions() != this->crossSection.getResolution())
		this->crossSection.generate(stem->getSectionDivisions());
	if (isFork)
		createFork(stem, state);
	else
		createBranchCollar(state, parentSegment);

	size_t sections = getSectionCount(stem, hasFork);
	for (; state.section < sections; state.section++) {
		Quat rotation = rotateSection(state);
		state.prevIndex = this->vertices[state.mesh].size();
		addSection(state, rotation, this->crossSection);
		if (state.section+1 < sections)
			addTriangleRing(
				state.prevIndex,
				this->vertices[state.mesh].size(),
				stem->getSectionDivisions(),
				state.mesh);
	}

	if (hasFork && state.section < stem->getPath().getSize()) {
		addTriangleRing(
			state.prevIndex,
			this->vertices[state.mesh].size(),
			stem->getSectionDivisions(),
			state.mesh);
		Quat rotation = rotateSection(state);
		state.prevIndex = this->vertices[state.mesh].size();
		size_t section1 = state.section;
		size_t section2 = stem->getPath().getSize() - 1;
		state.texOffset += getTextureLength(stem, section1, section2);
		state.section = section2;
		addSection(state, rotation, this->crossSection);
	} else if (!hasFork && stem->getMinRadius() > 0)
		capStem(stem, state.mesh, state.prevIndex);
}

/** The cross section is rotated so that the first point is always the topmost
point relative to the parent stem direction. */
void Mesh::setInitialRotation(Stem *stem, State &state)
{
	float position = stem->getDistance();
	Path parentPath = stem->getParent()->getPath();
	Vec3 parentDirection;
	parentDirection = parentPath.getIntermediateDirection(position);
	Path path = stem->getPath();
	Vec3 stemDirection = path.getDirection(0);
	Vec3 up(0.0f, 1.0f, 0.0f);
	state.prevRotation = pg::rotateIntoVecQ(up, stemDirection);
	state.prevDirection = stemDirection;

	Vec3 sideways(1.0f, 0.0f, 0.0f);
	sideways = pg::rotate(state.prevRotation, sideways);
	sideways = pg::normalize(sideways);
	up = pg::projectOntoPlane(parentDirection, stemDirection);
	up = pg::normalize(up);
	Quat rotation = pg::rotateIntoVecQ(sideways, up);
	state.prevRotation = rotation * state.prevRotation;
}

/** A rotation for a cross section is relative to the rotation of the previous
cross section. Twisting can occur in stems if all sections are rotated relative
to the global axis. */
Quat Mesh::rotateSection(State &state)
{
	Stem *stem = state.segment.stem;
	Vec3 direction = stem->getPath().getAverageDirection(state.section);
	Quat rotation = rotateIntoVecQ(state.prevDirection, direction);
	rotation *= state.prevRotation;
	state.prevRotation = rotation;
	state.prevDirection = direction;
	return rotation;
}

/** Generate a cross section for a point in the stem's path. Indices are added
at a latter stage to connect the sections. */
void Mesh::addSection(State &state, Quat rotation, const CrossSection &section)
{
	Stem *stem = state.segment.stem;
	DVertex vertex;
	vertex.uv.y = getTextureLength(stem, state.section) + state.texOffset;
	state.texOffset = vertex.uv.y;
	Vec3 location = stem->getLocation();
	location += stem->getPath().get(state.section);

	Vec2 indices;
	Vec2 weights;
	if (stem->getJoints().size() > 0)
		updateJointState(state, indices, weights);
	else {
		indices = Vec2(state.jointID, 0.0f);
		weights = Vec2(1.0f, 0.0f);
	}

	float radius = this->plant->getRadius(stem, state.section);
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
		this->vertices[state.mesh].push_back(vertex);
	}
}

float getAspect(const Plant *plant, const Stem *stem)
{
	float aspect = 1.0f;
	long materialID = stem->getMaterial(Stem::Outer);
	if (materialID > 0) {
		Material m = plant->getMaterial(materialID);
		aspect = m.getRatio();
	}
	return aspect;
}

/** Determine a length to preserve the aspect ratio throughout the stem. */
float Mesh::getTextureLength(Stem *stem, size_t section)
{
	if (section > 0) {
		float length = stem->getPath().getSegmentLength(section);
		float radius = this->plant->getRadius(stem, section - 1);
		float aspect = getAspect(this->plant, stem);
		return (length * aspect) / (radius * 2.0f * PI);
	} else
		return 0.0f;
}

float Mesh::getTextureLength(Stem *stem, size_t section1, size_t section2)
{
	Vec3 p1 = stem->getPath().get(section1);
	Vec3 p2 = stem->getPath().get(section2);
	float length = pg::magnitude(p2 - p1);
	float radius = this->plant->getRadius(stem, section1);
	float aspect = getAspect(this->plant, stem);
	return (length * aspect) / (radius * 2.0f * PI);
}

/** Compute indices between the cross section just generated and the cross
section that still needs to be generated. */
void Mesh::addTriangleRing(
	size_t prevIndex, size_t index, int divisions, int mesh)
{
	for (int i = 0; i <= divisions - 1; i++) {
		addTriangle(mesh, index, index + 1, prevIndex);
		index++;
		addTriangle(mesh, prevIndex, index, prevIndex + 1);
		prevIndex++;
	}
}

bool Mesh::addForks(Stem *fork[2], State state)
{
	size_t sections[2];
	getForkStart(fork, sections);
	Vec3 direction1 = fork[0]->getPath().get(sections[0]);
	Vec3 direction2 = fork[1]->getPath().get(sections[1]);
	if (pg::isZero(direction1) || pg::isZero(direction2))
		return false;
	else {
		direction1 = pg::normalize(direction1);
		direction2 = pg::normalize(direction2);
	}

	if (dot(direction1, state.prevDirection) <= 0)
		return false;
	if (dot(direction2, state.prevDirection) <= 0)
		return false;
	if (fork[0]->getPath().getSize() <= sections[0])
		return false;
	if (fork[1]->getPath().getSize() <= sections[1])
		return false;

	Segment segments[2];
	State fs;
	fs.section = sections[0];
	fs.texOffset = state.texOffset;
	fs.prevRotation = pg::rotateIntoVecQ(state.prevDirection, direction1);
	fs.prevRotation *= state.prevRotation;
	fs.prevDirection = direction1;
	segments[0] = addStem(fork[0], fs, state, true);
	fs.section = sections[1];
	fs.texOffset = state.texOffset;
	fs.prevRotation = pg::rotateIntoVecQ(state.prevDirection, direction2);
	fs.prevRotation *= state.prevRotation;
	fs.prevDirection = direction2;
	segments[1] = addStem(fork[1], fs, state, true);

	int divisions = state.segment.stem->getSectionDivisions();
	int midpoint = getForkMidpoint(
		divisions * 2, state.prevRotation, state.prevDirection,
		direction1, direction2);
	int length = divisions / 2;
	int offset;
	int edge1;
	int edge2;
	int quad = divisions % 4 == 0;
	if ((quad && midpoint % 2 == 0) || (!quad && midpoint % 2 != 0)) {
		midpoint /= 2;
		offset = 1;
		edge1 = midpoint - length / 2;
		edge2 = edge1 + length;
		edge1 += (edge1 < 0) * divisions;
		edge2 -= (edge2 >= divisions) * divisions;
	} else {
		midpoint /= 2;
		length = divisions / 2;
		offset = 0;
		edge1 = midpoint - length / 2 + quad;
		edge2 = midpoint + length / 2 + 1;
		length -= 1;
		edge1 += (edge1 < 0) * divisions;
		edge2 -= (edge2 >= divisions) * divisions;

		size_t mesh = fork[0]->getMaterial(Stem::Outer);
		int p = edge1 - 1;
		p += divisions * (p < 0);
		this->indices[mesh].push_back(segments[0].vertexStart + edge1);
		this->indices[mesh].push_back(segments[1].vertexStart + p);
		this->indices[mesh].push_back(segments[0].vertexStart + p);
		p = edge2 - 1;
		p += divisions * (p < 0);
		this->indices[mesh].push_back(segments[1].vertexStart + edge2);
		this->indices[mesh].push_back(segments[0].vertexStart + p);
		this->indices[mesh].push_back(segments[1].vertexStart + p);
	}

	Plane plane1;
	plane1.point = fork[0]->getLocation();
	plane1.normal = pg::normalize(0.5f*(state.prevDirection+direction1));
	Plane plane2;
	plane2.point = fork[1]->getLocation();
	plane2.normal = pg::normalize(0.5f*(state.prevDirection+direction2));
	Plane plane3;
	plane3.point = plane1.point;
	{
		Vec3 n1 = pg::normalize(pg::cross(direction2, direction1));
		Vec3 n2 = pg::cross(n1, direction2);
		Vec3 n3 = pg::cross(n1, direction1);
		plane3.normal = pg::normalize(n2 + n3);
	}
	DVertex *v0;
	DVertex *v1;
	DVertex *v2;
	{
		size_t start = state.segment.vertexStart;
		start += state.segment.vertexCount - divisions - 1;
		size_t mesh = fork[0]->getMaterial(Stem::Outer);
		v0 = &this->vertices[state.mesh][start];
		v1 = &this->vertices[mesh][segments[0].vertexStart];
		v2 = &this->vertices[mesh][segments[1].vertexStart];
	}

	Ray ray;
	for (int i = 0; i <= length; i++) {
		int k = edge1 + i;
		k -= (k >= divisions) * divisions;
		ray.origin = v0[k].position;
		ray.direction = state.prevDirection;
		float t = pg::intersectsPlane(ray, plane1);
		v0[k].position = ray.origin + t*ray.direction;
		v1[k].position = v0[k].position;
		v0[k].normal = 0.5f * (v0[k].normal + v1[k].normal);
		v1[k].normal = v0[k].normal;
		v0[k].uv.y += t;
		v1[k].uv.y = v0[k].uv.y;
	}
	for (int i = offset; i <= length - offset; i++) {
		int k = edge2 + i;
		k -= (k >= divisions) * divisions;
		ray.origin = v0[k].position;
		ray.direction = state.prevDirection;
		float t = pg::intersectsPlane(ray, plane2);
		v0[k].position = ray.origin + t*ray.direction;
		v2[k].position = v0[k].position;
		v0[k].normal = 0.5f * (v0[k].normal + v2[k].normal);
		v2[k].normal = v0[k].normal;
		v0[k].uv.y += t;
		v2[k].uv.y = v0[k].uv.y;
	}
	if (offset > 0) {
		v2[edge2].position = v1[edge2].position;
		v2[edge1].position = v1[edge1].position;
		v2[edge2].normal = v1[edge2].normal;
		v2[edge1].normal = v1[edge1].normal;
	}
	for (int i = offset; i <= length - offset; i++) {
		int k1 = edge1 - 1 + offset - i;
		int k2 = edge1 + i;
		k1 += (k1 < 0) * divisions;
		k1 -= (k1 >= divisions) * divisions;
		k2 += (k2 < 0) * divisions;
		k2 -= (k2 >= divisions) * divisions;
		ray.origin = v1[k1].position;
		ray.direction = direction1;
		float t = pg::intersectsPlane(ray, plane3);
		v1[k1].position = ray.origin + t*ray.direction;
		v2[k2].position = v1[k1].position;
		v1[k1].normal = 0.5f * (v1[k1].normal + v2[k2].normal);
		v2[k2].normal = v1[k1].normal;
		v1[k1].uv.y += t;
		v2[k2].uv.y = v1[k1].uv.y;
	}
	v0[divisions].position = v0[0].position;
	v1[divisions].position = v1[0].position;
	v2[divisions].position = v2[0].position;
	v0[divisions].normal = v0[0].normal;
	v1[divisions].normal = v1[0].normal;
	v2[divisions].normal = v2[0].normal;

	return true;
}

void Mesh::createFork(Stem *stem, State &state)
{
	Quat rotation = state.prevRotation;
	size_t section = state.section;
	state.section = 0;
	addSection(state, rotation, this->crossSection);
	state.section = section;
	state.texOffset += getTextureLength(stem, 0, section - 1);
	addTriangleRing(
		state.prevIndex,
		this->vertices[state.mesh].size(),
		stem->getSectionDivisions(),
		state.mesh);
}

/** Return the point index that is between the dividing line of the section. */
int Mesh::getForkMidpoint(int divisions, Quat rotation, Vec3 direction,
	Vec3 direction1, Vec3 direction2)
{
	int midpoint;
	Vec3 normal = pg::normalize(0.5f * (direction1 + direction2));
	rotation = rotateIntoVecQ(direction, normal) * rotation;
	direction1 = pg::rotate(pg::conjugate(rotation), direction1);
	Vec3 n = pg::projectOntoPlane(direction1, Vec3(0.0f, 1.0f, 0.0f));
	n = pg::normalize(n);
	float delta = 2.0f * PI / divisions;
	float theta = std::acos(n.x);
	if (n.z < 0.0f)
		theta = 2.0f*PI-theta;
	if (divisions % 4 == 0)
		midpoint = std::round(theta / delta);
	else
		midpoint = std::floor(theta / delta);
	midpoint -= (midpoint >= divisions) * divisions;
	return midpoint;
}

/** Return the first path indices for cross sections that do not intersect. */
void Mesh::getForkStart(Stem *fork[2], size_t sections[2])
{
	float radius = fork[0]->getMaxRadius();
	Path paths[2] = {fork[0]->getPath(), fork[1]->getPath()};
	sections[0] = 1;
	sections[1] = 1;
	size_t prevSections[2] = {1, 1};
	size_t index1 = 0;
	size_t index2 = 1;
	bool finished = false;

	while (sections[index1] < paths[index1].getSize()) {
		Vec3 p1 = paths[index1].get(sections[index1]);
		bool withinSegment = false;
		bool withinDiameter = false;
		float t = 0.0f;

		while (prevSections[index2] <= sections[index2]) {
			Vec3 p2a = paths[index2].get(prevSections[index2]-1);
			Vec3 p2b = paths[index2].get(prevSections[index2]);
			float len = pg::magnitude(p2b-p2a);
			t = pg::project(p1-p2a, pg::normalize(p2b-p2a));
			Vec3 p3 = t * pg::normalize(p2b-p2a) + p2a;
			withinSegment = t < len+radius && t > -radius;
			withinDiameter = pg::magnitude(p3-p1) < 2.0f*radius;
			if (withinSegment && withinDiameter)
				break;
			else
				prevSections[index2]++;
		}

		if (withinSegment) {
			if (withinDiameter) {
				sections[index1]++;
				finished = false;
			} else if (finished) {
				break;
			} else {
				finished = true;
				size_t t = index1;
				index1 = index2;
				index2 = t;
			}
		} else if (finished) {
			break;
		} else {
			finished = true;
			size_t t = index1;
			index1 = index2;
			index2 = t;
		}
	}
}

/** Create two cross sections and connect them with Bezier curves. */
void Mesh::createBranchCollar(State &state, Segment parentSegment)
{
	Stem *stem = state.segment.stem;
	Vec2 swelling = stem->getSwelling();
	state.section = 0;
	state.texOffset = 0.0f;
	State originalState = state;
	if (swelling.x < 1.0f || swelling.y < 1.0f)
		return;

	addSection(state, rotateSection(state), this->crossSection);
	size_t start = this->vertices[state.mesh].size();
	reserveBranchCollarSpace(stem, state.mesh);
	state.prevIndex = this->vertices[state.mesh].size();
	state.texOffset = 0.0f;
	state.section = stem->getPath().getDivisions() + 1;
	state.prevIndex = this->vertices[state.mesh].size();
	addSection(state, rotateSection(state), this->crossSection);

	state.section = connectCollar(state.segment, parentSegment, start);
	size_t sections = stem->getPath().getSize();
	if (state.section > 0 && state.section < sections)
		addTriangleRing(
			state.prevIndex,
			this->vertices[state.mesh].size(),
			stem->getSectionDivisions(),
			state.mesh);
	else if (state.section == 0)
		state = originalState;
}

/** Cross sections are usually created one at a time and then connected with
triangles. Branch collars are created by connecting cross sections with
splines, which means that many cross sections are created at a time. Reserving
memory in advance enables offsets to be used to maintain an identical vertex
layout. */
void Mesh::reserveBranchCollarSpace(Stem *stem, int mesh)
{
	size_t size = getBranchCollarSize(stem) + this->vertices[mesh].size();
	this->vertices[mesh].resize(size);
}

/** Return the amount of memory needed for the branch collar. */
size_t Mesh::getBranchCollarSize(Stem *stem)
{
	return (stem->getSectionDivisions()+1) * stem->getCollarDivisions();
}

/** The first step in generating the branch collar is scaling the first cross
section of the stem along the parent stem. */
Mat4 Mesh::getBranchCollarScale(Stem *child, Stem *parent)
{
	Mat4 scale = identity();

	if (!parent) {
		scale[0][0] = child->getSwelling().x;
		scale[2][2] = child->getSwelling().y;
		return scale;
	}

	scale[2][2] = child->getSwelling().x;
	scale[1][1] = child->getSwelling().y;

	float position = child->getDistance();
	Vec3 yaxis = parent->getPath().getIntermediateDirection(position);
	Vec3 xaxis = child->getPath().getDirection(0);
	xaxis = normalize(cross(cross(yaxis, xaxis), yaxis));
	Vec3 zaxis = normalize(cross(yaxis, xaxis));

	Mat4 axes = identity();
	axes.vectors[0] = toVec4(xaxis, 0.0f);
	axes.vectors[1] = toVec4(yaxis, 0.0f);
	axes.vectors[2] = toVec4(zaxis, 0.0f);

	return axes * scale * transpose(axes);
}

/** Project a point from a cross section on its parent's surface. */
DVertex Mesh::moveToSurface(DVertex vertex, Ray ray, Segment parent)
{
	float length = magnitude(ray.direction);
	ray.direction = normalize(ray.direction);

	if (!parent.stem) {
		Plane plane;
		plane.normal = Vec3(0.0f, 1.0f, 0.0f);
		plane.point = Vec3(0.0f, 0.0f, 0.0f);
		float t = -intersectsPlane(ray, plane);
		Vec3 offset = (t - length) * ray.direction;
		vertex.position -= offset;
		return vertex;
	}

	const unsigned mesh = parent.stem->getMaterial(Stem::Outer);
	float t = std::numeric_limits<float>::max();
	unsigned i = parent.indexStart;
	while (i < parent.indexStart + parent.indexCount) {
		unsigned index;
		index = this->indices[mesh][i++];
		Vec3 p1 = this->vertices[mesh][index].position;
		index = this->indices[mesh][i++];
		Vec3 p2 = this->vertices[mesh][index].position;
		index = this->indices[mesh][i++];
		Vec3 p3 = this->vertices[mesh][index].position;

		float s = -intersectsTriangle(ray, p1, p3, p2);
		if (s != 0 && s < t) {
			t = s;
			vertex.normal = cross(p1-p2, p1-p3);
		}
	}

	if (t != std::numeric_limits<float>::max()) {
		Vec3 offset = (t - length) * ray.direction;
		vertex.normal = normalize(vertex.normal);
		vertex.position -= offset;
	} else {
		vertex.position.x = std::numeric_limits<float>::infinity();
		vertex.position.y = std::numeric_limits<float>::infinity();
		vertex.position.z = std::numeric_limits<float>::infinity();
	}

	return vertex;
}

size_t Mesh::connectCollar(Segment child, Segment parent, size_t vertexStart)
{
	const unsigned mesh = child.stem->getMaterial(Stem::Outer);
	const int sectionDivisions = child.stem->getSectionDivisions();
	const int collarDivisions = child.stem->getCollarDivisions();
	const int pathDivisions = child.stem->getPath().getDivisions();
	size_t collarSize = getBranchCollarSize(child.stem);
	Mat4 scale = getBranchCollarScale(child.stem, parent.stem);

	int degree;
	Vec3 direction;
	Spline spline = child.stem->getPath().getSpline();
	degree = spline.getDegree();
	if (degree == 3) {
		auto controls = spline.getControls();
		direction = controls[3] - controls[2];
	}

	for (int i = 0; i <= sectionDivisions; i++) {
		size_t index = child.vertexStart + i;
		size_t nextIndex = index + collarSize + sectionDivisions + 1;

		Ray ray;
		Vec3 location = child.stem->getLocation();
		DVertex initPoint = this->vertices[mesh][index];
		DVertex scaledPoint;

		scaledPoint.position = initPoint.position - location;
		scaledPoint.position = scale.apply(scaledPoint.position, 1.0f);
		scaledPoint.position += location;
		ray.origin = this->vertices[mesh][nextIndex].position;
		ray.direction = ray.origin - scaledPoint.position;
		scaledPoint = moveToSurface(scaledPoint, ray, parent);
		if (std::isinf(scaledPoint.position.x)) {
			this->vertices[mesh].resize(child.vertexStart);
			this->indices[mesh].resize(child.indexStart);
			return 0;
		}
		scaledPoint.weights = this->vertices[mesh][index].weights;
		scaledPoint.indices = this->vertices[mesh][index].indices;
		this->vertices[mesh][index] = scaledPoint;

		ray.direction = ray.origin - initPoint.position;
		initPoint = moveToSurface(initPoint, ray, parent);
		if (std::isinf(initPoint.position.x)) {
			this->vertices[mesh].resize(child.vertexStart);
			this->indices[mesh].resize(child.indexStart);
			return 0;
		}

		spline.clear();
		spline.setDegree(3);
		spline.addControl(scaledPoint.position);
		spline.addControl(initPoint.position);
		Vec3 point = this->vertices[mesh][nextIndex].position;
		if (degree == 3)
			spline.addControl(point - direction);
		else
			spline.addControl(point);
		spline.addControl(point);

		float delta = 1.0f/(collarDivisions+1);
		float t = delta;
		index = vertexStart + i;
		for (int j = 0; j < collarDivisions; j++, t += delta) {
			DVertex vertex;
			vertex.position = spline.getPoint(0, t);
			vertex.indices = scaledPoint.indices;
			vertex.weights = scaledPoint.weights;
			size_t offset = index + (sectionDivisions + 1) * j;
			this->vertices[mesh][offset] = vertex;
		}
	}

	size_t index1 = child.vertexStart;
	size_t index2 = child.vertexStart + sectionDivisions + 1;
	for (int i = 0; i <= collarDivisions; i++) {
		addTriangleRing(index1, index2, sectionDivisions, mesh);
		index1 = index2;
		index2 += sectionDivisions + 1;
	}

	index1 = child.vertexStart;
	index2 = vertexStart + collarSize;
	setBranchCollarNormals(
		index1, index2, mesh, sectionDivisions, collarDivisions);
	setBranchCollarUVs(
		index2, child.stem, mesh, sectionDivisions, collarDivisions);
	return pathDivisions + 2;
}

/** Interpolate normals from the first cross section of the stem with normals
from the first cross section after the branch collar. */
void Mesh::setBranchCollarNormals(
	size_t index1, size_t index2, int mesh, int resolution, int divisions)
{
	for (int i = 0; i <= resolution; i++) {
		Vec3 normal1 = this->vertices[mesh][index1].normal;
		Vec3 normal2 = this->vertices[mesh][index2].normal;

		for (int j = 1; j <= divisions; j++) {
			float t = j/static_cast<float>(divisions);
			Vec3 normal = lerp(normal1, normal2, t);
			normal = normalize(normal);
			size_t offset = j * (resolution + 1);
			this->vertices[mesh][index1 + offset].normal = normal;
		}

		index1++;
		index2++;
	}
}

/** Normally UV coordinates are generated starting at the first cross section.
The UV coordinates for branch collars are generated backwards because splines
are not guaranteed to be the same length. */
void Mesh::setBranchCollarUVs(
	size_t lastIndex, Stem *stem, int mesh, int resolution, int divisions)
{
	size_t size = resolution + 1;
	float radius = this->plant->getRadius(stem, 1);
	float aspect = getAspect(this->plant, stem);

	for (int i = 0; i <= resolution; i++) {
		Vec2 uv = this->vertices[mesh][lastIndex + i].uv;
		size_t index = lastIndex + i;

		for (int j = divisions; j >= 0; j--) {
			Vec3 p1 = this->vertices[mesh][index].position;
			index -= size;
			Vec3 p2 = this->vertices[mesh][index].position;
			float length = magnitude(p2 - p1);
			uv.y -= (length * aspect) / (radius * 2.0f * PI);
			this->vertices[mesh][index].uv = uv;
		}
	}
}

void Mesh::capStem(Stem *stem, int stemMesh, size_t section)
{
	long mesh = stem->getMaterial(Stem::Inner);
	size_t index = section;
	size_t divisions = stem->getSectionDivisions();
	float rotation = 2.0f * PI / divisions;
	float angle = 0.0f;
	section = this->vertices[mesh].size();

	for (size_t i = 0; i <= divisions; i++, index++, angle += rotation) {
		DVertex vertex = this->vertices[stemMesh][index];
		vertex.uv.x = std::cos(angle) * 0.5f + 0.5f;
		vertex.uv.y = std::sin(angle) * 0.5f + 0.5f;
		this->vertices[mesh].push_back(vertex);
	}

	for (index = 0; index < divisions/2 - 1; index++) {
		addTriangle(mesh,
			section + index,
			section + divisions - index - 1,
			section + index + 1);
		addTriangle(mesh,
			section + index + 1,
			section + divisions - index - 1,
			section + divisions - index - 2);
	}

	if ((divisions & 1) != 0) { /* is odd */
		size_t lastSection = section + index;
		addTriangle(mesh, lastSection, lastSection+2, lastSection+1);
	}
}

void Mesh::addLeaves(Stem *stem, const State &state)
{
	auto leaves = stem->getLeaves();
	unsigned index = 0;
	for (auto it = leaves.begin(); it != leaves.end(); it++)
		addLeaf(stem, index++, state);
}

void Mesh::addLeaf(Stem *stem, unsigned leafIndex, const State &state)
{
	Leaf *leaf = stem->getLeaf(leafIndex);
	long mesh = leaf->getMaterial();
	Segment leafSegment;
	leafSegment.leafIndex = leafIndex;
	leafSegment.stem = stem;
	leafSegment.vertexStart = this->vertices[mesh].size();
	leafSegment.indexStart = this->indices[mesh].size();

	Vec2 weights;
	Vec2 indices;
	if (stem->hasJoints()) {
		float position = leaf->getPosition();
		auto pair = getJoint(position, stem);
		size_t index = pair.first;
		size_t pathIndex = pair.second.getPathIndex();
		float jointPosition = stem->getPath().getDistance(pathIndex);
		float offset = position - jointPosition;
		setJointInfo(stem, offset, index, weights, indices);
	} else {
		weights.x = 1.0f;
		weights.y = 0.0f;
		indices.x = static_cast<float>(state.jointID);
		indices.y = indices.x;
	}

	Geometry geom = transformLeaf(leaf, stem);
	size_t vsize = this->vertices[mesh].size();
	for (DVertex vertex : geom.getPoints()) {
		vertex.indices = indices;
		vertex.weights = weights;
		this->vertices[mesh].push_back(vertex);
	}
	for (unsigned i : geom.getIndices())
		this->indices[mesh].push_back(i + vsize);

	leafSegment.vertexCount = this->vertices[mesh].size();
	leafSegment.vertexCount -= leafSegment.vertexStart;
	leafSegment.indexCount = this->indices[mesh].size();
	leafSegment.indexCount -= leafSegment.indexStart;
	this->leafSegments[mesh].emplace(LeafID(stem, leafIndex), leafSegment);
}

Geometry Mesh::transformLeaf(const Leaf *leaf, const Stem *stem)
{
	const Path path = stem->getPath();
	Vec3 location = stem->getLocation();
	float position = leaf->getPosition();
	Vec3 direction;

	if (position >= 0.0f && position < path.getLength()) {
		direction = path.getIntermediateDirection(position);
		location += path.getIntermediate(position);
	} else {
		direction = path.getDirection(path.getSize() - 1);
		location += path.get().back();
	}

	Geometry geom = this->plant->getLeafMesh(leaf->getMesh());
	Quat rotation;
	rotation = leaf->getDefaultOrientation(direction);
	rotation *= leaf->getRotation();
	geom.transform(rotation, leaf->getScale(), location);
	return geom;
}

/** Stem descendants might not have joints and the parent state is needed to
determine what joint ancestors are influenced by. */
void Mesh::setInitialJointState(State &state, const State &parentState)
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

pair<size_t, Joint> Mesh::getJoint(float position, const Stem *stem)
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

void Mesh::incrementJoint(State &state, const vector<Joint> &joints)
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
void Mesh::updateJointState(State &state, Vec2 &indices, Vec2 &weights)
{
	const Stem *stem = state.segment.stem;
	const Path path = stem->getPath();
	const vector<Joint> joints = stem->getJoints();
	incrementJoint(state, joints);
	size_t pathIndex = joints[state.jointIndex].getPathIndex();

	if (state.jointIndex == 0 && state.section <= pathIndex) {
		weights.x = 1.0f;
		weights.y = 0.0f;
		indices.x = static_cast<float>(state.jointID);
		indices.y = indices.x;
	} else if (state.section == 0 || state.section == path.getSize()-1) {
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
		if (state.section != pathIndex) {
			Vec3 point1 = stem->getPath().get(state.section);
			Vec3 point2 = stem->getPath().get(state.section - 1);
			state.jointOffset += magnitude(point1-point2);
		}
		setJointInfo(stem, state.jointOffset, state.jointIndex,
			weights, indices);
	}
}

void Mesh::setJointInfo(const Stem *stem, float jointOffset, size_t jointIndex,
	Vec2 &weights, Vec2 &indices)
{
	const vector<Joint> joints = stem->getJoints();
	const Path path = stem->getPath();
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

void Mesh::addTriangle(int mesh, int a, int b, int c)
{
	this->indices[mesh].push_back(a);
	this->indices[mesh].push_back(b);
	this->indices[mesh].push_back(c);
}

void Mesh::initBuffer()
{
	size_t size = this->plant->getMaterials().size();
	this->vertices.resize(size);
	this->indices.resize(size);
	this->stemSegments.resize(size);
	this->leafSegments.resize(size);
	for (size_t i = 0; i < size; i++) {
		this->vertices[i].clear();
		this->indices[i].clear();
		this->stemSegments[i].clear();
		this->leafSegments[i].clear();
	}
}

/** Geometry is divided into different groups depending on material.
Geometry is latter stored in the same vertex buffer but is separated based
on material to minimize draw calls. This method updates the indices to what
they should be in the final merged vertex buffer. */
void Mesh::updateSegments()
{
	if (!this->indices.empty()) {
		unsigned vsize = this->vertices[0].size();
		unsigned isize = this->indices[0].size();
		for (unsigned mesh = 1; mesh < this->indices.size(); mesh++) {
			for (unsigned &index : this->indices[mesh])
				index += vsize;
			for (auto &pair : this->stemSegments[mesh]) {
				Segment *segment = &pair.second;
				segment->vertexStart += vsize;
				segment->indexStart += isize;
			}
			for (auto &pair : this->leafSegments[mesh]) {
				Segment *segment = &pair.second;
				segment->vertexStart += vsize;
				segment->indexStart += isize;
			}
			vsize += this->vertices[mesh].size();
			isize += this->indices[mesh].size();
		}
	}
}

size_t Mesh::getMeshCount() const
{
	return this->indices.size();
}

size_t Mesh::getVertexCount() const
{
	size_t size = 0;
	for (auto &mesh : this->vertices)
		size += mesh.size();
	return size;
}

size_t Mesh::getIndexCount() const
{
	size_t size = 0;
	for (auto &mesh : this->indices)
		size += mesh.size();
	return size;
}

unsigned Mesh::getMaterialIndex(int mesh) const
{
	return mesh;
}

vector<DVertex> Mesh::getVertices() const
{
	vector<DVertex> object;
	for (auto it = this->vertices.begin(); it != this->vertices.end(); it++)
		object.insert(object.end(), it->begin(), it->end());
	return object;
}

vector<unsigned> Mesh::getIndices() const
{
	vector<unsigned> object;
	for (auto it = this->indices.begin(); it != this->indices.end(); it++)
		object.insert(object.end(), it->begin(), it->end());
	return object;
}

const vector<DVertex> *Mesh::getVertices(int mesh) const
{
	return &this->vertices.at(mesh);
}

const vector<unsigned> *Mesh::getIndices(int mesh) const
{
	return &this->indices.at(mesh);
}

map<Mesh::LeafID, Segment> Mesh::getLeaves(int mesh) const
{
	return this->leafSegments.at(mesh);
}

size_t Mesh::getLeafCount(int mesh) const
{
	return this->leafSegments.at(mesh).size();
}

Segment Mesh::findStem(Stem *stem) const
{
	Segment segment = {};
	for (size_t i = 0; i < this->stemSegments.size(); i++) {
		try {
			segment = this->stemSegments[i].at(stem);
			break;
		} catch (std::out_of_range) {}
	}
	return segment;
}

Segment Mesh::findLeaf(LeafID leaf) const
{
	Segment segment = {};
	for (size_t i = 0; i < this->leafSegments.size(); i++) {
		try {
			segment = this->leafSegments[i].at(leaf);
			break;
		} catch (std::out_of_range) {}
	}
	return segment;
}

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
	addStem(stem);
	updateSegments();
}

bool Mesh::hasValidLocation(Stem *stem)
{
	return !std::isnan(stem->getLocation().x);
}

Segment Mesh::addStem(Stem *stem)
{
	State state;
	state.mesh = selectBuffer(stem->getMaterial(Stem::Outer));
	state.segment.stem = stem;
	state.segment.vertexStart = vertices[state.mesh].size();
	state.segment.indexStart = indices[state.mesh].size();
	addSections(state);
	state.segment.vertexCount = vertices[state.mesh].size();
	state.segment.vertexCount -= state.segment.vertexStart;
	state.segment.indexCount = indices[state.mesh].size();
	state.segment.indexCount -= state.segment.indexStart;
	this->stemSegments[state.mesh].emplace(stem->getID(), state.segment);

	addLeaves(stem);

	Stem *child = stem->getChild();
	while (child != nullptr) {
		if (hasValidLocation(child))
			addStem(child);
		child = child->getSibling();
	}

	return state.segment;
}

void Mesh::addSections(State &state)
{
	Stem *stem = state.segment.stem;
	Quat rotation;
	state.uvOffset = 0.0f;
	state.prevDirection = {0.0f, 1.0f, 0.0f};
	state.prevRotation = {0.0f, 0.0f, 0.0f, 1.0f};
	state.prevIndex = this->vertices[state.mesh].size();
	state.section = createBranchCollar(state);

	size_t sections = stem->getPath().getSize();
	for (; state.section < sections; state.section++) {
		rotation = rotateSection(state);
		state.prevIndex = this->vertices[state.mesh].size();
		addSection(state, rotation);

		if (state.section+1 < sections) {
			size_t i = this->vertices[state.mesh].size();
			int r = stem->getResolution();
			addTriangleRing(state.prevIndex, i, r, state.mesh);
		}
	}

	capStem(stem, state.mesh, state.prevIndex);
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
void Mesh::addSection(State &state, Quat rotation)
{
	Stem *stem = state.segment.stem;
	const float deltaAngle = 2.0f * PI / stem->getResolution();
	const float uOffset = 1.0f / stem->getResolution();
	float radius = stem->getPath().getRadius(state.section);
	float length = stem->getPath().getIntermediateDistance(state.section);
	float angle = 0.0f;
	float aspect = 1.0f;
	long materialID = stem->getMaterial(Stem::Outer);
	if (materialID > 0) {
		Material m = this->plant->getMaterial(materialID);
		aspect = m.getRatio();
	}

	Vertex vertex;
	vertex.uv.x = 1.0f;
	vertex.uv.y = -(length*aspect)/(radius*2.0f*PI)+state.uvOffset;
	state.uvOffset = vertex.uv.y;

	Vec3 location = stem->getLocation();
	location += stem->getPath().get(state.section);

	for (int i = 0; i <= stem->getResolution(); i++) {
		vertex.position = {std::cos(angle), 0.0f, std::sin(angle)};
		vertex.normal = normalize(vertex.position);
		vertex.position = radius * vertex.position;
		vertex.position = rotate(rotation, vertex.position, 1.0f);
		vertex.position += location;
		vertex.normal = rotate(rotation, vertex.normal, 0.0f);
		vertex.normal = normalize(vertex.normal);
		this->vertices[state.mesh].push_back(vertex);
		vertex.uv.x -= uOffset;
		angle += deltaAngle;
	}
}

/** Compute indices between the cross section just generated (starting at the
previous index) and the cross-section that still needs to be generated
(starting at the current index). */
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

size_t Mesh::createBranchCollar(State &state)
{
	Stem *stem = state.segment.stem;

	Vec2 swelling = stem->getSwelling();
	if (!(stem->getParent() && swelling.x > 1.0f && swelling.y > 1.0f))
		return 0;

	state.section = 0;
	state.prevIndex = this->vertices[state.mesh].size();
	addSection(state, rotateSection(state));

	size_t collarStart = this->vertices[state.mesh].size();
	reserveBranchCollarSpace(stem, state.mesh);
	state.prevIndex = this->vertices[state.mesh].size();
	state.uvOffset = 0.0f;
	state.section = 1;
	state.prevIndex = this->vertices[state.mesh].size();
	addSection(state, rotateSection(state));

	Segment parentSegment = findStem(stem->getParent());
	if (!connectCollar(state.segment, parentSegment, collarStart))
		return 0;
	else
		return 1;
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
	const int divisions = stem->getPath().getResolution();
	return (stem->getResolution()+1) * divisions;
}

/** The first step in generating the branch collar is scaling the first cross
section of the stem. This method returns the quantity to scale by. */
Mat4 Mesh::getBranchCollarScale(Stem *child, Stem *parent)
{
	float position = child->getPosition();
	Vec3 yaxis = parent->getPath().getIntermediateDirection(position);
	Vec3 xaxis = child->getPath().getDirection(0);
	xaxis = normalize(cross(cross(yaxis, xaxis), yaxis));
	Vec3 zaxis = normalize(cross(yaxis, xaxis));

	Mat4 axes = identity();
	axes.vectors[0] = toVec4(xaxis, 0.0f);
	axes.vectors[1] = toVec4(yaxis, 0.0f);
	axes.vectors[2] = toVec4(zaxis, 0.0f);

	Mat4 scale = identity();
	scale[2][2] = child->getSwelling().x;
	scale[1][1] = child->getSwelling().y;

	return axes * scale * transpose(axes);
}

/** Project a point from a cross section on its parent's surface. */
Vertex Mesh::moveToSurface(Vertex vertex, Ray ray, Segment parent, int mesh)
{
	float length = magnitude(ray.direction);
	ray.direction = normalize(ray.direction);

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

bool Mesh::connectCollar(Segment child, Segment parent, size_t vertexStart)
{
	const int mesh1 = selectBuffer(child.stem->getMaterial(Stem::Outer));
	const int mesh2 = selectBuffer(parent.stem->getMaterial(Stem::Outer));
	const int resolution = child.stem->getResolution();
	const int divisions = child.stem->getPath().getResolution();
	size_t collarSize = getBranchCollarSize(child.stem);
	Mat4 scale = getBranchCollarScale(child.stem, parent.stem);

	for (int i = 0; i <= resolution; i++) {
		size_t index = child.vertexStart + i;
		size_t nextIndex = index + collarSize + resolution + 1;

		Ray ray;
		Vec3 location = child.stem->getLocation();
		Vertex initPoint = this->vertices[mesh1][index];
		Vertex scaledPoint;

		scaledPoint.position = initPoint.position - location;
		scaledPoint.position = scale.apply(scaledPoint.position, 1.0f);
		scaledPoint.position += location;
		ray.origin = this->vertices[mesh1][nextIndex].position;
		ray.direction = ray.origin - scaledPoint.position;
		scaledPoint = moveToSurface(scaledPoint, ray, parent, mesh2);
		if (std::isinf(scaledPoint.position.x)) {
			this->vertices[mesh1].resize(child.vertexStart);
			this->indices[mesh1].resize(child.indexStart);
			return false;
		}
		this->vertices[mesh1][index] = scaledPoint;

		ray.direction = ray.origin - initPoint.position;
		initPoint = moveToSurface(initPoint, ray, parent, mesh2);
		if (std::isinf(initPoint.position.x)) {
			this->vertices[mesh1].resize(child.vertexStart);
			this->indices[mesh1].resize(child.indexStart);
			return false;
		}

		Spline spline;
		spline.setDegree(3);
		spline.addControl(scaledPoint.position);
		spline.addControl(initPoint.position);
		spline.addControl(initPoint.position);
		spline.addControl(this->vertices[mesh1][nextIndex].position);

		float delta = 1.0f/(divisions+1);
		float t = delta;
		for (int j = 0; j < divisions; j++, t += delta) {
			Vertex vertex;
			vertex.position = spline.getPoint(0, t);
			size_t index = vertexStart + i + (resolution + 1) * j;
			this->vertices[mesh1][index] = vertex;
		}
	}

	size_t index1 = child.vertexStart;
	size_t index2 = child.vertexStart + resolution + 1;
	for (int i = 0; i <= divisions; i++) {
		addTriangleRing(index1, index2, resolution, mesh1);
		index1 = index2;
		index2 += resolution + 1;
	}

	index1 = child.vertexStart;
	index2 = vertexStart + collarSize;
	setBranchCollarNormals(index1, index2, mesh1, resolution, divisions);
	setBranchCollarUVs(index2, child.stem, mesh1, resolution, divisions);
	return true;
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
	float radius = stem->getPath().getRadius(1);

	for (int i = 0; i <= resolution; i++) {
		Vec2 uv = this->vertices[mesh][lastIndex + i].uv;
		size_t index = lastIndex + i;

		for (int j = divisions; j >= 0; j--) {
			Vec3 p1 = this->vertices[mesh][index].position;
			index -= size;
			Vec3 p2 = this->vertices[mesh][index].position;
			float length = magnitude(p2 - p1);
			uv.y += length/(radius * 2.0f * PI);
			this->vertices[mesh][index].uv = uv;
		}
	}
}

void Mesh::capStem(Stem *stem, int stemMesh, size_t section)
{
	int mesh = selectBuffer(stem->getMaterial(Stem::Inner));
	int index = section;
	int divisions = stem->getResolution();
	float rotation = 2.0f * PI / divisions;
	float angle = 0.0f;
	section = this->vertices[mesh].size();

	for (int i = 0; i <= divisions; i++, index++, angle += rotation) {
		Vertex vertex = this->vertices[stemMesh][index];
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

void Mesh::addLeaves(Stem *stem)
{
	auto leaves = stem->getLeaves();
	for (auto it = leaves.begin(); it != leaves.end(); it++) {
		const Leaf *leaf = stem->getLeaf(it->first);
		addLeaf(leaf, stem);
	}
}

void Mesh::addLeaf(const Leaf *leaf, Stem *stem)
{
	int mesh = selectBuffer(leaf->getMaterial());

	Segment leafSegment;
	leafSegment.leaf = leaf->getID();
	leafSegment.stem = stem;
	leafSegment.vertexStart = this->vertices[mesh].size();
	leafSegment.indexStart = this->indices[mesh].size();

	Geometry geom = transformLeaf(leaf, stem);
	size_t index = this->vertices[mesh].size();
	for (Vertex vertex : geom.getPoints())
		this->vertices[mesh].push_back(vertex);
	for (unsigned i : geom.getIndices())
		this->indices[mesh].push_back(i + index);

	leafSegment.vertexCount = vertices[mesh].size();
	leafSegment.vertexCount -= leafSegment.vertexStart;
	leafSegment.indexCount = indices[mesh].size();
	leafSegment.indexCount -= leafSegment.indexStart;
	leafSegments[mesh].emplace(leaf->getID(), leafSegment);
}

Geometry Mesh::transformLeaf(const Leaf *leaf, const Stem *stem)
{
	Path path = stem->getPath();
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

void Mesh::addTriangle(int mesh, int a, int b, int c)
{
	this->indices[mesh].push_back(a);
	this->indices[mesh].push_back(b);
	this->indices[mesh].push_back(c);
}

/** Different buffers are used for different materials. This is done to keep
geometry with identical materials together and simplify draw calls. */
int Mesh::selectBuffer(long material)
{
	auto it = this->meshes.find(material);
	int mesh = it != this->meshes.end() ? it->first : 0;
	if (material != 0) {
		if (it == this->meshes.end()) {
			this->vertices.push_back(vector<Vertex>());
			this->indices.push_back(vector<unsigned>());
			this->stemSegments.push_back(map<long, Segment>());
			this->leafSegments.push_back(map<long, Segment>());
			this->meshes[material] = this->indices.size() - 1;
			this->materials.push_back(material);
		}
		mesh = this->meshes[material];
	}
	return mesh;
}

void Mesh::initBuffer()
{
	this->materials.clear();
	this->vertices.clear();
	this->indices.clear();
	this->meshes.clear();
	this->stemSegments.clear();
	this->leafSegments.clear();

	this->materials.push_back(0);
	this->vertices.push_back(vector<Vertex>());
	this->indices.push_back(vector<unsigned>());
	this->stemSegments.push_back(map<long, Segment>());
	this->leafSegments.push_back(map<long, Segment>());
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

int Mesh::getMeshCount() const
{
	return indices.size();
}

int Mesh::getVertexCount() const
{
	int size = 0;
	for (auto &mesh : this->vertices)
		size += mesh.size();
	return size;
}

int Mesh::getIndexCount() const
{
	int size = 0;
	for (auto &mesh : this->indices)
		size += mesh.size();
	return size;
}

long Mesh::getMaterialID(int mesh) const
{
	return this->materials.at(mesh);
}

vector<Vertex> Mesh::getVertices() const
{
	vector<Vertex> object;
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

const vector<Vertex> *Mesh::getVertices(int mesh) const
{
	return &this->vertices[mesh];
}

const vector<unsigned> *Mesh::getIndices(int mesh) const
{
	return &this->indices[mesh];
}

map<long, Segment> Mesh::getLeaves(int mesh) const
{
	return this->leafSegments[mesh];
}

int Mesh::getLeafCount(int mesh) const
{
	return this->leafSegments[mesh].size();
}

Segment Mesh::findStem(Stem *stem) const
{
	Segment segment = {};
	for (size_t i = 0; i < this->stemSegments.size(); i++) {
		try {
			segment = this->stemSegments[i].at(stem->getID());
			break;
		} catch (std::out_of_range) {}
	}
	return segment;
}

Segment Mesh::findLeaf(long leaf) const
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

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

using pg::Vec3;
using pg::Mat4;
using pg::Stem;
using pg::Segment;
using pg::Mesh;
using std::map;
using std::vector;

Mesh::Mesh(pg::Plant *plant)
{
	this->plant = plant;
	this->defaultLeaf.setPerpendicularPlanes();
}

void Mesh::generate()
{
	Stem *stem = this->plant->getRoot();
	initBuffer();
	addStem(stem);
	updateSegments();
}

Segment Mesh::addStem(Stem *stem)
{
	/* Hide stems with invalid locations. */
	if (std::isnan(stem->getLocation().x))
		return (Segment){};

	int mesh = selectBuffer(stem->getMaterial(Stem::Outer));
	this->mesh = mesh;

	Segment stemSegment;
	stemSegment.stem = stem;
	stemSegment.vertexStart = vertices[mesh].size();
	stemSegment.indexStart = indices[mesh].size();
	float uvOffset = 0.0f;
	int lastSection = stem->getPath().getSize() - 1;

	size_t prevIndex = vertices[mesh].size();
	addSection(stem, 0, uvOffset);
	size_t currentIndex = vertices[mesh].size();
	addTriangleRing(prevIndex, currentIndex, stem->getResolution());

	for (int section = 1; section < lastSection; section++) {
		size_t prevIndex = vertices[mesh].size();
		addSection(stem, section, uvOffset);
		size_t currentIndex = vertices[mesh].size();
		addTriangleRing(prevIndex, currentIndex, stem->getResolution());
	}

	size_t lastSectionIndex = vertices[mesh].size();
	addSection(stem, lastSection, uvOffset);
	this->mesh = selectBuffer(stem->getMaterial(Stem::Inner));
	capStem(stem, mesh, lastSectionIndex);

	stemSegment.vertexCount = vertices[mesh].size();
	stemSegment.vertexCount -= stemSegment.vertexStart;
	stemSegment.indexCount = indices[mesh].size();
	stemSegment.indexCount -= stemSegment.indexStart;
	this->stemSegments[mesh].emplace(stem->getID(), stemSegment);

	addLeaves(stem);

	Stem *child = stem->getChild();
	while (child != nullptr) {
		addStem(child);
		child = child->getSibling();
	}

	return stemSegment;
}

/** Generate a cross-section for a point in the stem's path. Indices are added
at a latter stage (two cross-sections are required to form a ring of
triangles). */
void Mesh::addSection(Stem *stem, size_t section, float &uvOffset)
{
	Mat4 transform = getSectionTransform(stem, section);
	float radius = stem->getPath().getRadius(section);
	float length = stem->getPath().getIntermediateDistance(section);
	float angle = 0.0f;
	float rotation = 2.0f * M_PI / stem->getResolution();
	float uOffset = 1.0f / stem->getResolution();

	Vertex vertex;
	vertex.uv = {1.0f, -length/(radius * 2.0f * (float)M_PI) + uvOffset};
	uvOffset = vertex.uv.y;

	for (int i = 0; i <= stem->getResolution(); i++) {
		vertex.position = {std::cos(angle), 0.0f, std::sin(angle)};
		vertex.normal = normalize(vertex.position);
		vertex.position = radius * vertex.position;
		vertex.position = transform.apply(vertex.position, 1.0f);
		vertex.normal = transform.apply(vertex.normal, 0.0f);
		vertex.normal = normalize(vertex.normal);

		this->vertices[this->mesh].push_back(vertex);

		vertex.uv.x -= uOffset;
		angle += rotation;
	}
}

/** Transform points in the cross-section so that they face the direction of
the stem's path. */
Mat4 Mesh::getSectionTransform(Stem *stem, size_t section)
{
	Vec3 up = {0.0f, 1.0f, 0.0f};
	Vec3 point = stem->getPath().get(section);
	Vec3 direction = stem->getPath().getAverageDirection(section);
	Vec3 location = stem->getLocation() + point;
	Mat4 rotation = rotateIntoVec(up, direction);
	Mat4 translation = translate(location);
	return translation * rotation;
}

/** Compute indices between the cross-section just generated (starting at the
previous index) and the cross-section that still needs to be generated
(which will start at the current index). */
void Mesh::addTriangleRing(size_t prevIndex, size_t index, int divisions)
{
	for (int i = 0; i <= divisions - 1; i++) {
		addTriangle(index, index + 1, prevIndex);
		index++;
		addTriangle(prevIndex, index, prevIndex + 1);
		prevIndex++;
	}
}

void Mesh::capStem(Stem *stem, int stemMesh, size_t section)
{
	int index = section;
	int divisions = stem->getResolution();
	float rotation = 2.0f * M_PI / divisions;
	float angle = 0.0f;
	section = this->vertices[this->mesh].size();

	for (int i = 0; i <= divisions; i++, index++, angle += rotation) {
		Vertex vertex = this->vertices[stemMesh][index];
		vertex.uv.x = std::cos(angle) * 0.5f + 0.5f;
		vertex.uv.y = std::sin(angle) * 0.5f + 0.5f;
		this->vertices[this->mesh].push_back(vertex);
	}

	for (index = 0; index < divisions/2 - 1; index++) {
		addTriangle(
			section + index,
			section + divisions - index - 1,
			section + index + 1);
		addTriangle(
			section + index + 1,
			section + divisions - index - 1,
			section + divisions - index - 2);
	}

	if ((divisions & 1) != 0) { /* is odd */
		size_t lastSection = section + index;
		addTriangle(lastSection, lastSection + 2, lastSection + 1);
	}
}

void Mesh::addLeaves(Stem *stem)
{
	auto leaves = stem->getLeaves();
	for (auto it = leaves.begin(); it != leaves.end(); it++) {
		Leaf *leaf = stem->getLeaf(it->first);
		this->mesh = selectBuffer(leaf->getMaterial());
		addLeaf(leaf, stem);
	}
}

void Mesh::addLeaf(Leaf *leaf, Stem *stem)
{
	Segment leafSegment;
	leafSegment.leaf = leaf->getID();
	leafSegment.stem = stem;
	leafSegment.vertexStart = this->vertices[this->mesh].size();
	leafSegment.indexStart = this->indices[this->mesh].size();

	Path path = stem->getPath();
	Vec3 location = stem->getLocation();
	float position = leaf->getPosition();
	Vec3 direction;
	Quat rotation;

	if (position >= 0.0f && position < path.getLength()) {
		direction = path.getIntermediateDirection(position);
		location += path.getIntermediate(position);
	} else {
		direction = path.getDirection(path.getSize() - 1);
		location += path.get().back();
	}

	Geometry geom = this->defaultLeaf;
	if (leaf->getMesh() != 0)
		geom = this->plant->getLeafMesh(leaf->getMesh());
	rotation = leaf->getDefaultOrientation(direction);
	rotation *= leaf->getRotation();
	geom.transform(rotation, leaf->getScale(), location);

	size_t index = this->vertices[this->mesh].size();
	for (Vertex vertex : geom.getPoints())
		this->vertices[this->mesh].push_back(vertex);
	for (unsigned i : geom.getIndices())
		this->indices[this->mesh].push_back(i + index);

	leafSegment.vertexCount = vertices[this->mesh].size();
	leafSegment.vertexCount -= leafSegment.vertexStart;
	leafSegment.indexCount = indices[this->mesh].size();
	leafSegment.indexCount -= leafSegment.indexStart;
	leafSegments[this->mesh].emplace(leaf->getID(), leafSegment);
}

void Mesh::addTriangle(int a, int b, int c)
{
	this->indices[this->mesh].push_back(a);
	this->indices[this->mesh].push_back(b);
	this->indices[this->mesh].push_back(c);
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

/** Geometry is divided into different groups depending on its material.
Geometry is latter on stored in the same vertex buffer, but is seperated based
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

vector<pg::Vertex> Mesh::getVertices() const
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

const vector<pg::Vertex> *Mesh::getVertices(int mesh) const
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

/** Find the location of a stem in the buffer. */
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

/** Find the location of a leaf in the buffer. */
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

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

using namespace pg;
using std::pair;
using std::vector;

const float pi = 3.14159265359f;

Mesh::Mesh(Plant *plant) : plant(plant)
{

}

size_t Mesh::insertTriangleRing(
	size_t i1, size_t i2, int divisions, unsigned *indices)
{
	size_t index = 0;
	for (int i = 0; i < divisions; i++) {
		indices[index++] = i2;
		indices[index++] = i2 + 1;
		indices[index++] = i1;
		i2++;
		indices[index++] = i1;
		indices[index++] = i2;
		indices[index++] = i1 + 1;
		i1++;
	}
	return index;
}

/** Compute indices between the cross section just generated and the cross
section that still needs to be generated. Counterclockwise is outward. */
void Mesh::addTriangleRing(size_t i1, size_t i2, int divisions, int mesh)
{
	for (int i = 0; i < divisions; i++) {
		addTriangle(mesh, i1, i2 + 1, i2);
		i2++;
		addTriangle(mesh, i1, i1 + 1, i2);
		i1++;
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
	this->stems.resize(size);
	this->leaves.resize(size);
	for (size_t i = 0; i < size; i++) {
		this->vertices[i].clear();
		this->indices[i].clear();
		this->stems[i].clear();
		this->leaves[i].clear();
	}
}

/** Geometry is divided into different groups depending on material.
Geometry is later stored in the same vertex buffer but is separated based
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
			for (auto &pair : this->stems[mesh]) {
				Segment *segment = &pair.second;
				segment->vertexStart += vsize;
				segment->indexStart += isize;
			}
			for (auto &pair : this->leaves[mesh]) {
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

std::map<Mesh::LeafID, Mesh::Segment> Mesh::getLeaves(int mesh) const
{
	return this->leaves.at(mesh);
}

size_t Mesh::getLeafCount(int mesh) const
{
	return this->leaves.at(mesh).size();
}

Mesh::Segment Mesh::findStem(Stem *stem) const
{
	for (size_t i = 0; i < this->stems.size(); i++) {
		auto it = this->stems[i].find(stem);
		if (it != this->stems[i].end())
			return it->second;
	}
	return Segment();
}

Mesh::Segment Mesh::findLeaf(LeafID leaf) const
{
	for (size_t i = 0; i < this->leaves.size(); i++) {
		auto it = this->leaves[i].find(leaf);
		if (it != this->leaves[i].end())
			return it->second;
	}
	return Segment();
}

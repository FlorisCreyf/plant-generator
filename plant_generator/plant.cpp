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

#include "plant.h"

using namespace pg;
using std::vector;
using std::pair;

Plant::Plant()
{
	this->root = nullptr;
}

Plant::~Plant()
{
	deallocateStems(this->root);
}

Stem *Plant::move(Stem *value)
{
	Stem *stem = this->stemPool.allocate();
	Stem *childValue = value->getChild();
	*stem = *value;
	stem->child = nullptr;
	stem->parent = nullptr;
	stem->nextSibling = nullptr;
	stem->prevSibling = nullptr;
	while (childValue) {
		Stem *siblingValue = childValue->nextSibling;
		Stem *child = move(childValue);
		insertStem(child, stem);
		childValue = siblingValue;
	}
	delete value;
	return stem;
}

StemPool *Plant::getStemPool()
{
	return &this->stemPool;
}

Stem *Plant::addStem(Stem *parent)
{
	if (!parent)
		return createRoot();
	else {
		Stem *stem = this->stemPool.allocate();
		stem->init(parent);
		Stem *firstChild = parent->child;
		parent->child = stem;
		if (firstChild)
			firstChild->prevSibling = stem;
		stem->nextSibling = firstChild;
		stem->prevSibling = nullptr;
		return stem;
	}
}

Stem *Plant::createRoot()
{
	if (this->root)
		deallocateStems(this->root);
	this->root = this->stemPool.allocate();
	this->root->init(nullptr);
	return this->root;
}

void Plant::decouple(Stem *stem)
{
	if (stem == this->root)
		this->root = nullptr;
	if (stem->prevSibling)
		stem->prevSibling->nextSibling = stem->nextSibling;
	if (stem->nextSibling)
		stem->nextSibling->prevSibling = stem->prevSibling;
	if (stem->parent && stem->parent->child == stem) {
		if (stem->prevSibling)
			stem->parent->child = stem->prevSibling;
		else
			stem->parent->child = stem->nextSibling;
	}
}

Stem *Plant::getRoot()
{
	return this->root;
}

const Stem *Plant::getRoot() const
{
	return this->root;
}

void Plant::updateDepth(Stem *stem, int depth)
{
	stem->depth = depth;
	Stem *child = stem->getChild();
	while (child) {
		updateDepth(child, depth + 1);
		child = child->getSibling();
	}
}

void Plant::insertStem(Stem *stem, Stem *parent)
{
	updateDepth(stem, parent->getDepth() + 1);
	Stem *firstChild = parent->child;
	parent->child = stem;
	stem->parent = parent;
	if (firstChild)
		firstChild->prevSibling = parent->child;
	parent->child->nextSibling = firstChild;
	parent->child->prevSibling = nullptr;
}

void Plant::removeRoot()
{
	if (this->root) {
		deallocateStems(this->root);
		this->root = nullptr;
	}
}

void Plant::deallocateStems(Stem *stem)
{
	Stem *child = stem->child;
	while (child) {
		Stem *next = child->nextSibling;
		deallocateStems(child);
		child = next;
	}
	this->stemPool.deallocate(stem);
}

void Plant::deleteStem(Stem *stem)
{
	decouple(stem);
	deallocateStems(stem);
}

void Plant::copy(vector<pair<Stem *, Stem>> &stems, Stem *stem)
{
	stems.push_back(pair<Stem *, Stem>(stem, *stem));
	Stem *child = stem->getChild();
	while (child) {
		copy(stems, child);
		child = child->getSibling();
	}
}

void Plant::extractStems(Stem *stem, vector<pair<Stem *, Stem>> &stems)
{
	stem->depth = 0;
	copy(stems, stem);
	deleteStem(stem);
}

void Plant::reinsertStems(vector<pair<Stem *, Stem>> &stems)
{
	for (auto instance : stems) {
		Stem *stem = instance.first;
		this->stemPool.allocateAt(stem);
		*stem = instance.second;
		if (stem->depth == 0 && stem->parent)
			insertStem(stem, stem->parent);
		else if (!stem->parent && !this->root)
			this->root = stem;
	}
}

void Plant::addMaterial(Material material)
{
	this->materials[material.getID()] = material;
}

void Plant::removeMaterial(long id)
{
	if (this->root) {
		if (this->root->getMaterial(Stem::Outer) == id)
			this->root->setMaterial(Stem::Outer, 0);
		if (this->root->getMaterial(Stem::Inner) == id)
			this->root->setMaterial(Stem::Inner, 0);
		removeMaterial(this->root, id);
	}
	this->materials.erase(id);
}

void Plant::removeMaterial(Stem *stem, long id)
{
	Stem *child = stem->child;
	while (child) {
		if (child->getMaterial(Stem::Outer) == id)
			child->setMaterial(Stem::Outer, 0);
		if (child->getMaterial(Stem::Inner) == id)
			child->setMaterial(Stem::Inner, 0);
		removeMaterial(child, id);
		child = child->nextSibling;
	}
}

Material Plant::getMaterial(long id) const
{
	return this->materials.at(id);
}

std::map<long, Material> Plant::getMaterials() const
{
	return this->materials;
}

void Plant::addLeafMesh(Geometry mesh)
{
	this->leafMeshes[mesh.getID()] = mesh;
}

void Plant::removeLeafMesh(long id)
{
	this->leafMeshes.erase(id);
}

void Plant::removeLeafMeshes()
{
	this->leafMeshes.clear();
}

Geometry Plant::getLeafMesh(long id) const
{
	if (id != 0)
		return this->leafMeshes.at(id);
	else {
		Geometry geom;
		geom.setPerpendicularPlanes();
		return geom;
	}
}

std::map<long, Geometry> Plant::getLeafMeshes() const
{
	return this->leafMeshes;
}

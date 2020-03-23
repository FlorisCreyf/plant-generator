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

using pg::Plant;
using pg::Stem;
using pg::Geometry;
using pg::Material;

Plant::Plant()
{
	this->root = nullptr;
}

Plant::~Plant()
{
	delete root;
}

Stem *Plant::addStem(Stem *parent)
{
	Stem *stem = new Stem(parent);
	Stem *firstChild = parent->child;
	parent->child = stem;
	if (firstChild)
		firstChild->prevSibling = stem;
	stem->nextSibling = firstChild;
	stem->prevSibling = nullptr;
	return stem;
}

Stem *Plant::createRoot()
{
	delete this->root;
	this->root = new Stem(nullptr);
	return this->root;
}

Stem *Plant::extractStem(pg::Stem *stem)
{
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
	return stem;
}

Stem *Plant::getRoot()
{
	return this->root;
}

void Plant::insertStem(Stem *stem, Stem *parent)
{
	Stem *firstChild = parent->child;
	parent->child = stem;
	if (firstChild)
		firstChild->prevSibling = parent->child;
	parent->child->nextSibling = firstChild;
	parent->child->prevSibling = nullptr;
}

void Plant::removeRoot()
{
	delete this->root;
	this->root = nullptr;
}

void Plant::addMaterial(Material material)
{
	materials[material.getID()] = material;
}

void Plant::removeMaterial(long id)
{
	if (root) {
		if (root->getMaterial(Stem::Outer) == id)
			root->setMaterial(Stem::Outer, 0);
		if (root->getMaterial(Stem::Inner) == id)
			root->setMaterial(Stem::Inner, 0);
		removeMaterial(root, id);
	}
	materials.erase(id);
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
	return materials.at(id);
}

std::map<long, Material> Plant::getMaterials() const
{
	return materials;
}

void Plant::addLeafMesh(Geometry mesh)
{
	leafMeshes[mesh.getID()] = mesh;
}

void Plant::removeLeafMesh(long id)
{
	leafMeshes.erase(id);
}

void Plant::removeLeafMeshes()
{
	leafMeshes.clear();
}

Geometry Plant::getLeafMesh(long id)
{
	return leafMeshes[id];
}

std::map<long, Geometry> Plant::getLeafMeshes()
{
	return leafMeshes;
}

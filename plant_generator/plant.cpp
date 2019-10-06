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

using pg::Stem;

pg::Plant::Plant()
{
	root = nullptr;
}

pg::Plant::~Plant()
{
	removeStem(root);
}

Stem *pg::Plant::getRoot()
{
	return root;
}

Stem *pg::Plant::addStem(Stem *stem)
{
	Stem *firstChild = stem->child;
	stem->child = new Stem(stem);
	if (firstChild)
		firstChild->prevSibling = stem->child;
	stem->child->nextSibling = firstChild;
	stem->child->prevSibling = nullptr;
	return stem->child;
}

void pg::Plant::removeFromTree(pg::Stem *stem)
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
}

void pg::Plant::deleteStem(Stem *stem)
{
	Stem *child = stem->child;
	while (child) {
		Stem *next = child->nextSibling;
		deleteStem(child);
		child = next;
	}
	delete stem;
}

void pg::Plant::removeStem(Stem *stem)
{
	if (stem->parent)
		deleteStem(stem);
}

void pg::Plant::insert(Stem *parent, Stem *child)
{
	Stem *firstChild = parent->child;
	parent->child = child;
	if (firstChild)
		firstChild->prevSibling = parent->child;
	parent->child->nextSibling = firstChild;
	parent->child->prevSibling = nullptr;
}

void pg::Plant::release(Stem *stem)
{
	if (stem->parent)
		removeFromTree(stem);
}

bool pg::Plant::contains(Stem *stem)
{
	return contains(stem, root);
}

bool pg::Plant::contains(Stem *a, Stem *b)
{
	if (a == b)
		return true;
	else if (b == nullptr)
		return false;
	else {
		Stem *child = b->child;
		while (child) {
			Stem *next = child->nextSibling;
			if (contains(a, child))
				return true;
			child = next;
		}
		return false;
	}
}

void pg::Plant::removeRoot()
{
	if (root != nullptr) {
		Stem *child = root->child;
		while (child) {
			Stem *next = child->nextSibling;
			removeStem(child);
			child = next;
		}
		delete root;
	}
	root = nullptr;
}

void pg::Plant::setRoot(Stem *stem)
{
	this->root = stem;
}

void pg::Plant::addMaterial(pg::Material material)
{
	materials[material.getId()] = material;
}

void pg::Plant::removeMaterial(unsigned id)
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

void pg::Plant::removeMaterial(Stem *stem, unsigned id)
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

pg::Material pg::Plant::getMaterial(unsigned id)
{
	return materials[id];
}

std::map<unsigned, pg::Material> pg::Plant::getMaterials()
{
	return materials;
}

void pg::Plant::addLeafMesh(pg::Geometry mesh)
{
	leafMeshes[mesh.getId()] = mesh;
}

pg::Geometry pg::Plant::getLeafMesh(unsigned id)
{
	return leafMeshes[id];
}

void pg::Plant::removeLeafMesh(unsigned id)
{
	leafMeshes.erase(id);
}

void pg::Plant::removeLeafMeshes()
{
	leafMeshes.clear();
}

std::map<unsigned, pg::Geometry> pg::Plant::getLeafMeshes()
{
	return leafMeshes;
}

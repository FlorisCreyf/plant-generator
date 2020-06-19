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
	if (this->root)
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

void Plant::insertStem(Stem *stem, Stem *parent)
{
	Stem *firstChild = parent->child;
	parent->child = stem;
	stem->parent = parent;
	if (firstChild)
		firstChild->prevSibling = stem;
	stem->nextSibling = firstChild;
	stem->prevSibling = nullptr;
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

void Plant::copy(vector<Plant::Extraction> &stems, Stem *stem)
{
	Extraction extraction;
	extraction.address = stem;
	extraction.value = *stem;
	extraction.parent = stem->parent;
	stems.push_back(extraction);
	Stem *child = stem->child;
	while (child) {
		copy(stems, child);
		child = child->nextSibling;
	}
}

Plant::Extraction Plant::extractStem(Stem *stem)
{
	Extraction extraction;
	extraction.address = stem;
	extraction.value = *stem;
	extraction.parent = stem->parent;
	deleteStem(stem);
	return extraction;
}

void Plant::extractStems(Stem *stem, vector<Plant::Extraction> &stems)
{
	copy(stems, stem);
	deleteStem(stem);
}

void Plant::reinsertStem(Extraction &extraction)
{
	this->stemPool.allocateAt(extraction.address);
	*extraction.address = extraction.value;
	extraction.address->child = nullptr;
	extraction.address->parent = nullptr;
	extraction.address->nextSibling = nullptr;
	extraction.address->prevSibling = nullptr;
	/* Assume that the parent stem is already inserted. */
	if (extraction.parent)
		insertStem(extraction.address, extraction.parent);
	else if (!extraction.parent && !this->root)
		this->root = extraction.address;
}

void Plant::reinsertStems(vector<Plant::Extraction> &extractions)
{
	for (Extraction &extraction : extractions)
		reinsertStem(extraction);
}

void Plant::addMaterial(Material material)
{
	this->materials.push_back(material);
}

void Plant::updateMaterial(Material material, unsigned index)
{
	this->materials[index] = material;
}

void Plant::removeMaterial(unsigned index)
{
	if (this->root)
		removeMaterial(this->root, index);
	this->materials.erase(this->materials.begin()+index);
}

void Plant::removeMaterial(Stem *stem, unsigned index)
{
	unsigned outerMaterial = stem->getMaterial(Stem::Outer);
	unsigned innerMaterial = stem->getMaterial(Stem::Inner);
	if (outerMaterial == index)
		stem->setMaterial(Stem::Outer, 0);
	else if (outerMaterial > index)
		stem->setMaterial(Stem::Outer, outerMaterial-1);
	if (innerMaterial == index)
		stem->setMaterial(Stem::Inner, 0);
	else if (innerMaterial > index)
		stem->setMaterial(Stem::Inner, innerMaterial-1);

	vector<Leaf> &leaves = stem->leaves;
	for (Leaf &leaf : leaves) {
		unsigned leafMaterial = leaf.getMaterial();
		if (leafMaterial == index)
			leaf.setMaterial(0);
		else if (leafMaterial > index)
			leaf.setMaterial(leafMaterial-1);
	}

	Stem *child = stem->child;
	while (child) {
		removeMaterial(child, index);
		child = child->nextSibling;
	}
}

Material Plant::getMaterial(unsigned index) const
{
	return this->materials.at(index);
}

std::vector<Material> Plant::getMaterials() const
{
	return this->materials;
}

void Plant::addLeafMesh(Geometry mesh)
{
	this->leafMeshes.push_back(mesh);
}

void Plant::updateLeafMesh(Geometry mesh, unsigned index)
{
	this->leafMeshes[index] = mesh;
}

void Plant::removeLeafMesh(unsigned index)
{
	if (this->root)
		removeLeafMesh(this->root, index);
	this->leafMeshes.erase(this->leafMeshes.begin()+index);
}

void Plant::removeLeafMesh(Stem *stem, unsigned index)
{
	vector<Leaf> &leaves = stem->leaves;
	for (Leaf &leaf : leaves) {
		unsigned mesh = leaf.getMesh();
		if (mesh == index)
			leaf.setMesh(0);
		else if (mesh > index)
			leaf.setMesh(mesh-1);
	}

	Stem *child = stem->child;
	while (child) {
		removeLeafMesh(child, index);
		child = child->nextSibling;
	}
}

void Plant::removeLeafMeshes()
{
	this->leafMeshes.clear();
}

Geometry Plant::getLeafMesh(unsigned index) const
{
	return this->leafMeshes.at(index);
}

std::vector<Geometry> Plant::getLeafMeshes() const
{
	return this->leafMeshes;
}

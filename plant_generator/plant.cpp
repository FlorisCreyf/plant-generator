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
#include <assert.h>

using namespace pg;
using std::vector;
using std::pair;

Plant::Plant() : root(nullptr)
{

}

Plant::~Plant()
{
	if (this->root)
		deallocateStems(this->root);
}

void Plant::setDefault()
{
	if (this->leafMeshes.empty()) {
		Geometry geometry;
		geometry.setPlane();
		this->addLeafMesh(geometry);
	}
	if (this->materials.empty())
		this->addMaterial(Material());
	if (this->curves.empty())
		this->addCurve(Curve(0));
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
		insertStem(child, stem, nullptr);
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
	if (stem == this->root) {
		assert(stem->prevSibling == nullptr);
		this->root = stem->nextSibling;
	}
	if (stem->prevSibling)
		stem->prevSibling->nextSibling = stem->nextSibling;
	if (stem->nextSibling)
		stem->nextSibling->prevSibling = stem->prevSibling;
	if (stem->parent && stem->parent->child == stem) {
		assert(stem->prevSibling == nullptr);
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

Stem *Plant::getLastSibling(Stem *stem)
{
	if (!stem)
		return nullptr;
	while (stem->nextSibling)
		stem = stem->nextSibling;
	return stem;
}

void Plant::insertStem(Stem *stem, Stem *parent, Stem *sibling)
{
	if (!sibling) {
		assert(parent);
		sibling = getLastSibling(parent->child);
		if (!sibling)
			insertStemAtBeginning(stem, parent);
		else
			insertStemAfterSibling(stem, parent, sibling);
	} else {
		assert(sibling->parent == parent);
		insertStemBeforeSibling(stem, parent, sibling);
	}
}

void Plant::insertStemAfterSibling(Stem *stem, Stem *parent, Stem *sibling)
{
	Stem *nextSibling = sibling->nextSibling;
	sibling->nextSibling = stem;
	stem->prevSibling = sibling;
	stem->nextSibling = nextSibling;
	stem->parent = parent;
	if (nextSibling)
		nextSibling->prevSibling = stem;
}

void Plant::insertStemBeforeSibling(Stem *stem, Stem *parent, Stem *sibling)
{
	Stem *prevSibling = sibling->prevSibling;
	sibling->prevSibling = stem;
	stem->prevSibling = prevSibling;
	stem->nextSibling = sibling;
	stem->parent = parent;
	if (prevSibling)
		prevSibling->nextSibling = stem;
	else
		parent->child = stem;
}

void Plant::insertStemAtBeginning(Stem *stem, Stem *parent)
{
	Stem *firstChild = parent->child;
	parent->child = stem;
	stem->nextSibling = firstChild;
	stem->prevSibling = nullptr;
	stem->parent = parent;
	if (firstChild)
		firstChild->prevSibling = stem;
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

void Plant::copy(vector<Stem> &stems, Stem *stem)
{
	Stem *child = stem->child;
	while (child) {
		copy(stems, child);
		child = child->nextSibling;
	}
	Stem extraction;
	extraction = *stem;
	extraction.nextSibling = stem->nextSibling;
	extraction.prevSibling = stem->prevSibling;
	extraction.parent = stem->parent;
	extraction.child = stem;
	stems.push_back(extraction);
}

Stem Plant::extractStem(Stem *stem)
{
	Stem extraction;
	extraction = *stem;
	extraction.nextSibling = stem->nextSibling;
	extraction.prevSibling = stem->prevSibling;
	extraction.parent = stem->parent;
	extraction.child = stem;
	deleteStem(stem);
	return extraction;
}

void Plant::extractStems(Stem *stem, vector<Stem> &stems)
{
	copy(stems, stem);
	deleteStem(stem);
}

void Plant::reinsertStem(Stem &extraction)
{
	Stem *stem = this->stemPool.allocate();
	assert(extraction.child == stem);

	*stem = extraction;
	stem->child = nullptr;
	stem->parent = nullptr;
	stem->nextSibling = nullptr;
	stem->prevSibling = nullptr;

	if (extraction.parent)
		insertStem(stem, extraction.parent, extraction.nextSibling);
	else if (!extraction.parent && !this->root)
		this->root = stem;
}

void Plant::reinsertStems(vector<Stem> &extractions)
{
	auto it = extractions.rbegin();
	auto end = extractions.rend();
	for (; it != end; it++)
		reinsertStem(*it);
}

float Plant::getRadius(Stem *stem, unsigned index) const
{
	float t = stem->path.getPercentage(index);
	const Spline spline = this->curves[stem->getRadiusCurve()].getSpline();
	float z = spline.getPoint(t).y;
	return z * (stem->maxRadius - stem->minRadius) + stem->minRadius;
}

float Plant::getIntermediateRadius(Stem *stem, float t) const
{
	float length = stem->path.getLength();
	const Spline spline = this->curves[stem->getRadiusCurve()].getSpline();
	float z = spline.getPoint(t / length).y;
	return z * (stem->maxRadius - stem->minRadius) + stem->minRadius;
}

void Plant::addCurve(Curve curve)
{
	this->curves.push_back(curve);
}

void Plant::updateCurve(Curve curve, unsigned index)
{
	this->curves[index] = curve;
}

void Plant::removeCurve(unsigned index)
{
	if (this->root)
		removeCurve(this->root, index);
	this->curves.erase(this->curves.begin()+index);
}

void Plant::removeCurve(Stem *stem, unsigned index)
{
	unsigned curve = stem->getRadiusCurve();
	if (index == curve)
		stem->setRadiusCurve(0);
	else if (curve > index)
		stem->setRadiusCurve(curve-1);

	Stem *child = stem->child;
	while (child) {
		removeCurve(child, index);
		child = child->nextSibling;
	}
}

Curve Plant::getCurve(unsigned index) const
{
	return this->curves.at(index);
}

const std::vector<Curve> &Plant::getCurves() const
{
	return this->curves;
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

const std::vector<Material> &Plant::getMaterials() const
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

const std::vector<Geometry> &Plant::getLeafMeshes() const
{
	return this->leafMeshes;
}

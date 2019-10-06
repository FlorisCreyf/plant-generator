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

#include "stem.h"
#include <limits>
#include <cmath>

unsigned pg::Stem::counter = 1;

pg::Stem::Stem(pg::Stem *parent)
{
	id = counter++;
	this->nextSibling = nullptr;
	this->prevSibling = nullptr;
	this->child = nullptr;
	this->parent = parent;
	if (parent == nullptr) {
		depth = 0;
		position = 0.0f;
		location = {0.0f, 0.0f, 0.0f};
	} else
		depth = parent->depth + 1;
}

bool pg::Stem::operator==(const pg::Stem &stem) const
{
	return (
		nextSibling == stem.nextSibling &&
		prevSibling == stem.prevSibling &&
		child == stem.child &&
		parent == stem.parent &&
		depth == stem.depth &&
		path == stem.path &&
		resolution == stem.resolution &&
		position == stem.position &&
		location == stem.location &&
		material[0] == stem.material[0] &&
		material[1] == stem.material[1] &&
		leaves == stem.leaves
	);
}

bool pg::Stem::operator!=(const pg::Stem &stem) const
{
	return !(*this == stem);
}

unsigned pg::Stem::getId() const
{
	return id;
}

int pg::Stem::addLeaf(const Leaf &leaf)
{
	leaves.emplace(leaf.getId(), leaf);
	return leaves.size() - 1;
}

int pg::Stem::getLeafCount()
{
	return leaves.size();
}

pg::Leaf *pg::Stem::getLeaf(int id)
{
	return &leaves.at(id);
}

const std::map<int, pg::Leaf> &pg::Stem::getLeaves()
{
	return leaves;
}

void pg::Stem::removeLeaf(int id)
{
	leaves.erase(leaves.find(id));
}

void pg::Stem::setResolution(int resolution)
{
	this->resolution = resolution;
}

int pg::Stem::getResolution() const
{
	return resolution;
}

void pg::Stem::setPath(pg::Path &path)
{
	this->path = path;
	updatePositions(this);
}

pg::Path pg::Stem::getPath()
{
	return path;
}

void pg::Stem::updatePositions(pg::Stem *stem)
{
	Stem *child = stem->child;
	while (child != nullptr) {
		child->setPosition(child->position);
		updatePositions(child);
		child = child->nextSibling;
	}
}

void pg::Stem::setPosition(float position)
{
	if (parent != nullptr) {
		Path parentPath = parent->getPath();
		pg::Vec3 point = parentPath.getIntermediate(position);
		if (std::isnan(point.x))
			location = point;
		else
			location = parent->getLocation() + point;
		this->position = position;
		updatePositions(this);
	}
}

float pg::Stem::getPosition() const
{
	return position;
}

pg::Vec3 pg::Stem::getLocation() const
{
	return location;
}

void pg::Stem::setMaterial(int feature, unsigned material)
{
	this->material[feature] = material;
}

unsigned pg::Stem::getMaterial(int feature) const
{
	return material[feature];
}

pg::Stem *pg::Stem::getParent()
{
	return parent;
}

pg::Stem *pg::Stem::getSibling()
{
	return nextSibling;
}

pg::Stem *pg::Stem::getChild()
{
	return child;
}

bool pg::Stem::isDescendantOf(pg::Stem *stem) const
{
	const Stem *descendant = this;
	if (depth > stem->getDepth()) {
		while (descendant != nullptr) {
			if (stem == descendant->parent)
				return true;
			descendant = descendant->parent;
		}
	}
	return false;
}

int pg::Stem::getDepth() const
{
	return depth;
}

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

pg::Stem::Stem(pg::Stem *parent)
{
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

void pg::Stem::setResolution(int resolution)
{
	this->resolution = resolution;
}

int pg::Stem::getResolution() const
{
	return resolution;
}

void pg::Stem::setPath(pg::VolumetricPath &path)
{
	this->path = path;
	updatePositions(this);
}

pg::VolumetricPath pg::Stem::getPath()
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
		VolumetricPath parentPath = parent->getPath();
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
	while (descendant != nullptr) {
		if (stem == descendant->parent)
			return true;
		descendant = stem->parent;
	}
	return false;
}

int pg::Stem::getDepth() const
{
	return depth;
}

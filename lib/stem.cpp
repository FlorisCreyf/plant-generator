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

Stem::Stem(unsigned name, Stem *parent)
{
	this->name = name++;
	this->parent = parent;
	if (parent == nullptr) {
		depth = 0;
		position = 0.0f;
		location = {0.0f, 0.0f, 0.0f};
	} else {
		depth = parent->depth + 1;
	}
}

unsigned Stem::getName()
{
	return name;
}

void Stem::modifyResolutions(Stem *stem, int resolution)
{
	if (hasDichotomous) {
		children[0].resolution = resolution;
		children[1].resolution = resolution;
		children[0].modifyResolutions(&children[0], resolution);
		children[1].modifyResolutions(&children[1], resolution);
	}
}

void Stem::updatePositions(Stem *stem)
{
	for (size_t i = 0; i < stem->children.size(); i++) {
		Stem *child = &stem->children[i];
		
		if (i < 2 && stem->hasDichotomous)
			child->setPosition(stem->path.getLength());
		else
			child->setPosition(child->position);
		updatePositions(child);
	}
}

void Stem::setPath(Path &path)
{	
	this->path = path;
	if (!this->path.isGenerated())
		this->path.generate();
	
	updatePositions(this);
}

void Stem::setPosition(float position)
{
	if (parent == nullptr)
		return;
		
	Path parentPath = parent->getPath();
	Vec3 point = parentPath.getPoint(position);
	if (point.x == std::numeric_limits<float>::infinity())
		location = point;
	else	
		location = parent->getLocation() + point;
	this->position = position;
	updatePositions(this);
}

void Stem::setLocation(Vec3 location)
{
	if (isLateral())
		return;
	
	if (parent != nullptr && this->path.isGenerated()) {
		auto c = parent->path.getControls();
		c[c.size() - 1] = location - parent->location;
		parent->path.setControls(c);
		parent->path.generate();			
		updatePositions(this->parent);
	} else if (parent == nullptr) {
		this->location = location;
		updatePositions(this);
	}
}

void Stem::setResolution(int resolution)
{
	if (parent != nullptr && !isLateral())
		parent->setResolution(resolution);
	else {
		this->resolution = resolution;
		modifyResolutions(this, resolution);
	}
}

bool Stem::isDescendantOf(Stem *stem)
{
	Stem *descendant = this;
	while (descendant != nullptr) {
		if (stem == descendant->parent)
			return true;
		descendant = stem->parent;
	}
	return false;
}

bool Stem::isLateral()
{
	if (depth == 0)
		return false;
	if (parent->hasDichotomousStems()) {
		if (parent->getDichotomousStem(0) == this)
			return false;
		if (parent->getDichotomousStem(1) == this)
			return false;
	}
	return true;
}

Stem *Stem::getParent()
{
	return parent;
}

Stem *Stem::getChild(size_t index)
{
	return &children[index + (hasDichotomous ? 2 : 0)];
}

size_t Stem::getChildCount()
{
	return children.size() - (hasDichotomous ? 2 : 0);
}

Stem *Stem::getDichotomousStem(size_t index)
{
	return &children[index];
}

bool Stem::hasDichotomousStems()
{
	return hasDichotomous;
}

Path Stem::getPath()
{
	return path;
}

float Stem::getPosition()
{
	return position;
}

Vec3 Stem::getLocation()
{
	return location;
}

int Stem::getResolution()
{
	return resolution;
}

int Stem::getDepth()
{
	return depth;
}

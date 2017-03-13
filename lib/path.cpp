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

#include "path.h"

using namespace treemaker;

float Path::getLength(std::vector<Vec3> &path)
{
	float total = 0.0f;
	for (size_t i = 0; i < path.size()-1; i++)
		total += magnitude(path[i+1] - path[i]);
	return total;
}

void Path::generateLinear()
{
	const float totalLength = getLength(controls);
	const float segment = totalLength / static_cast<float>(divisions - 1);
	size_t control = 1;
	float lineLength = magnitude(controls[0] - controls[1]);
	float lineDistance = 0.0f;
	
	path.clear();
	path.push_back(controls[0]);

	for (int i = 1; i < divisions - 1; i++) {
		Vec3 dir;
		float distance = segment + lineDistance;

		while (distance >= lineLength) {
			Vec3 diff;

			if (++control >= controls.size())
				return;
			

			distance -= lineLength;
			diff = controls[control] - controls[control-1];
			lineLength = magnitude(diff);
		}
		
		lineDistance = distance;
		dir = normalize(controls[control] - controls[control-1]);
		path.push_back(controls[control-1] + distance * dir);
	}
	
	path.push_back(controls.back());
}

void Path::generate()
{
	if (controls.size() < 2)
		return;
	
	switch (type) {
	case LINEAR:
		generateLinear();
		generated = true;
		break;
	case BEZIER:
		break;
	}
}

void Path::setControls(std::vector<Vec3> controls)
{
	controls[0] = {0.0f, 0.0f, 0.0f};
	this->controls = controls;
	generated = false;
}

std::vector<Vec3> Path::getControls()
{
	return controls;
}

void Path::setDivisions(int divisions)
{
	this->divisions = divisions;
	generated = false;
}

int Path::getDivisions()
{
	return divisions;
}

void Path::setType(Type type)
{
	this->type = type;
	generated = false;
}

Path::Type Path::getType()
{
	return type;
}

bool Path::isGenerated()
{
	return generated;
}

float Path::getLength()
{
	return getLength(path);
}

Vec3 Path::getDirection(int index)
{
	if (index == divisions - 1)
		return normalize(path[index] - path[index - 1]);
	else
		return normalize(path[index + 1] - path[index]);
}

Vec3 Path::getGeneratedPoint(int index)
{
	return path[index];
}

Vec3 Path::getPoint(float distance)
{
	float total = 0.0f;
	for (size_t i = 0; i < path.size()-1; i++) {
		float length = magnitude(path[i+1] - path[i]);
		if (total + length >= distance) {
			float t = distance - total;
			return t * getDirection(i) + path[i];
		}
		total += length;
	}
	
	Vec3 v = {};
	return v;
}

std::vector<Vec3> Path::getPath()
{
	return path;
}

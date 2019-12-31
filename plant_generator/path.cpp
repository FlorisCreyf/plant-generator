/* Copyright 2017-2018 Floris Creyf
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
#include <limits>

using pg::Path;
using pg::Spline;
using pg::Vec3;

bool Path::operator==(const Path &path) const
{
	return (
		this->path == path.path &&
		this->spline == path.spline &&
		this->resolution == path.resolution &&
		this->subdivisions == path.subdivisions &&
		this->radius == path.radius &&
		this->minRadius == path.minRadius &&
		this->maxRadius == path.maxRadius
	);
}

bool Path::operator!=(const Path &path) const
{
	return !(*this == path);
}

void Path::generate()
{
	size_t numControls = spline.getControls().size();
	if (numControls > 0) {
		path.clear();

		int curves = spline.getCurveCount();
		int curve = 0;
		for (curve = 0; curve < curves; curve++) {
			float r = 1.0f / resolution;
			float t = 0.0f;
			for (int i = 0; i < resolution; i++) {
				t = r * i;
				Vec3 point = spline.getPoint(curve, t);
				path.push_back(point);
			}
		}
		path.push_back(spline.getControls()[numControls-1]);
	}
}

void Path::setSpline(Spline &spline)
{
	this->spline = spline;
	generate();
}

Spline Path::getSpline()
{
	return spline;
}

/** Sets the divisions for each curve in the path. */
void Path::setResolution(int resolution)
{
	this->resolution = resolution;
	generate();
}

int Path::getResolution() const
{
	return resolution;
}

void Path::subdivide(int level)
{
	subdivisions = level;
	// TODO subdivide
}

int Path::getSubdivisions() const
{
	return subdivisions;
}

std::vector<Vec3> Path::get() const
{
	return path;
}

Vec3 Path::get(int index) const
{
	return path[index];
}

int Path::getSize() const
{
	return path.size();
}

Vec3 Path::getIntermediate(float distance) const
{
	Vec3 point = {
		std::numeric_limits<float>::quiet_NaN(),
		std::numeric_limits<float>::quiet_NaN(),
		std::numeric_limits<float>::quiet_NaN()
	};
	float total = 0.0f;

	for (size_t i = 0; i < path.size() - 1; i++) {
		float length = magnitude(path[i + 1] - path[i]);
		if (total + length >= distance) {
			point = (distance - total) * getDirection(i) + path[i];
			total += length;
			break;
		}
		total += length;
	}

	if (total < distance)
		point = path.back();

	return point;
}

float Path::getLength() const
{
	float length = 0.0f;
	for (size_t i = 0; i < path.size() - 1; i++)
		length += magnitude(path[i + 1] - path[i]);
	return length;
}

Vec3 Path::getDirection(size_t index) const
{
	if (index == path.size() - 1)
		return normalize(path[index] - path[index - 1]);
	else
		return normalize(path[index + 1] - path[index]);
}

Vec3 Path::getAverageDirection(size_t index) const
{
	Vec3 direction;
	if (index == 0 || index == path.size() - 1)
		direction = getDirection(index);
	else {
		direction = getDirection(index) + getDirection(index - 1);
		direction = normalize(direction);
	}
	return direction;
}

Vec3 Path::getIntermediateDirection(float t) const
{
	Vec3 direction;
	float s = 0.0f;

	for (size_t i = 0; i < path.size() - 1; i++) {
		s += magnitude(path[i+1] - path[i]);
		if (s >= t) {
			direction = getDirection(i);
			break;
		}
	}

	if (s < t)
		direction = getDirection(path.size() - 1);

	return direction;
}

float Path::getDistance(int index) const
{
	std::vector<Vec3> controls = spline.getControls();
	int lastIndex = index / spline.getDegree() * this->resolution;
	float distance = 0.0f;

	for (int i = 0; i < lastIndex; i++)
		distance += magnitude(path[i + 1] - path[i]);

	return distance;
}

float Path::getIntermediateDistance(int index) const
{
	if (index == 0)
		return 0.0f;
	else
		return magnitude(path[index] - path[index-1]);
}

void Path::setMaxRadius(float radius)
{
	maxRadius = radius;
}

float Path::getMaxRadius()
{
	return maxRadius;
}

void Path::setMinRadius(float radius)
{
	minRadius = radius;
}

float Path::getMinRadius()
{
	return minRadius;
}

void Path::setRadius(Spline spline)
{
	radius = spline;
}

Spline Path::getRadius()
{
	return radius;
}

float Path::getRadius(int index)
{
	float length = 0.0f;
	for (int i = 0; i < index; i++)
		length += magnitude(path[i+1] - path[i]);
	float t = length / getLength();
	float z = radius.getPoint(t).z * maxRadius;
	return z < minRadius ? minRadius : z;
}

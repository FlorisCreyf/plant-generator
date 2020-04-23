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
		this->maxRadius == path.maxRadius);
}

bool Path::operator!=(const Path &path) const
{
	return !(*this == path);
}

void Path::generate(bool linearStart)
{
	size_t numControls = this->spline.getControls().size();
	if (numControls <= 1)
	 	return;

	int curves = this->spline.getCurveCount();
	int curve = 0;
	this->path.clear();

	if (linearStart)
		this->path.push_back(this->spline.getPoint(curve++, 0.0f));

	while (curve < curves) {
		float r = 1.0f / resolution;
		float t = 0.0f;
		for (int i = 0; i < resolution; i++) {
			t = r * i;
			Vec3 point = this->spline.getPoint(curve, t);
			this->path.push_back(point);
		}
		curve++;
	}

	this->path.push_back(this->spline.getControls()[numControls-1]);
}

void Path::setSpline(Spline &spline)
{
	this->spline = spline;
}

Spline Path::getSpline()
{
	return this->spline;
}

/** Sets the divisions for each curve in the path. */
void Path::setResolution(int resolution)
{
	this->resolution = resolution;
}

int Path::getResolution() const
{
	return this->resolution;
}

void Path::subdivide(int level)
{
	this->subdivisions = level;
	/* TODO subdivide */
}

int Path::getSubdivisions() const
{
	return this->subdivisions;
}

std::vector<Vec3> Path::get() const
{
	return this->path;
}

Vec3 Path::get(int index) const
{
	return this->path[index];
}

int Path::getSize() const
{
	return this->path.size();
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
			point = (distance - total) * getDirection(i);
			point += this->path[i];
			total += length;
			break;
		}
		total += length;
	}

	if (total < distance)
		point = this->path.back();

	return point;
}

float Path::getLength() const
{
	float length = 0.0f;
	for (size_t i = 0; i < this->path.size() - 1; i++)
		length += magnitude(this->path[i + 1] - this->path[i]);
	return length;
}

Vec3 Path::getDirection(size_t index) const
{
	if (index == this->path.size() - 1)
		return normalize(this->path[index] - this->path[index - 1]);
	else
		return normalize(this->path[index + 1] - this->path[index]);
}

Vec3 Path::getAverageDirection(size_t index) const
{
	Vec3 direction;
	if (index == 0 || index == this->path.size() - 1)
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
	float dist = 0.0f;

	for (size_t i = 0; i < this->path.size() - 1; i++) {
		dist += magnitude(this->path[i+1] - this->path[i]);
		if (dist >= t) {
			direction = getDirection(i);
			break;
		}
	}

	if (dist < t)
		return getDirection(this->path.size() - 1);
	else
		return direction;
}

float Path::getDistance(int index) const
{
	std::vector<Vec3> controls = this->spline.getControls();
	int lastIndex = index / this->spline.getDegree() * this->resolution;
	float distance = 0.0f;
	for (int i = 0; i < lastIndex; i++)
		distance += magnitude(this->path[i + 1] - this->path[i]);
	return distance;
}

float Path::getIntermediateDistance(int index) const
{
	if (index == 0)
		return 0.0f;
	else
		return magnitude(this->path[index] - this->path[index-1]);
}

void Path::setMaxRadius(float radius)
{
	this->maxRadius = radius;
}

float Path::getMaxRadius() const
{
	return this->maxRadius;
}

void Path::setMinRadius(float radius)
{
	this->minRadius = radius;
}

float Path::getMinRadius() const
{
	return this->minRadius;
}

void Path::setRadius(Spline spline)
{
	this->radius = spline;
}

Spline Path::getRadius() const
{
	return this->radius;
}

float Path::getRadius(int index) const
{
	float length = 0.0f;
	for (int i = 0; i < index; i++)
		length += magnitude(this->path[i+1] - this->path[i]);
	float t = length / getLength();
	float z = this->radius.getPoint(t).z * this->maxRadius;
	return z < this->minRadius ? this->minRadius : z;
}

float Path::getIntermediateRadius(float t) const
{
	float z = this->radius.getPoint(t / getLength()).z * this->maxRadius;
	return z < this->minRadius ? this->minRadius : z;
}

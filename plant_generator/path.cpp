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

Path::Path()
{
	this->divisions = 0;
	this->initialDivisions = 0;
	this->subdivisions = 0;
	this->length = 0.0f;
}

bool Path::operator==(const Path &path) const
{
	return (
		this->path == path.path &&
		this->spline == path.spline &&
		this->divisions == path.divisions &&
		this->subdivisions == path.subdivisions);
}

bool Path::operator!=(const Path &path) const
{
	return !(*this == path);
}

void Path::generate()
{
	size_t size = this->spline.getControls().size();
	int curves = this->spline.getCurveCount();
	if (size <= 1)
	 	return;

	this->path.clear();

	float delta = 1.0f / (this->initialDivisions+1);
	for (int i = 0; i <= this->initialDivisions; i++) {
		float t = delta * i;
		Vec3 point = this->spline.getPoint(0, t);
		this->path.push_back(point);
	}

	delta = 1.0f / (this->divisions+1);
	for (int curve = 1; curve < curves; curve++) {
		for (int i = 0; i <= this->divisions; i++) {
			float t = delta * i;
			Vec3 point = this->spline.getPoint(curve, t);
			this->path.push_back(point);
		}
	}

	this->path.push_back(this->spline.getControls()[size-1]);
	setLength();
}

void Path::setLength()
{
	this->length = 0.0f;
	for (size_t i = 0; i < this->path.size() - 1; i++)
		this->length += magnitude(this->path[i + 1] - this->path[i]);
}

void Path::setSpline(const Spline &spline)
{
	this->spline = spline;
}

Spline Path::getSpline() const
{
	return this->spline;
}

void Path::setDivisions(int resolution)
{
	this->divisions = resolution;
}

int Path::getDivisions() const
{
	return this->divisions;
}

void Path::setInitialDivisions(int divisions)
{
	this->initialDivisions = divisions;
}

int Path::getInitialDivisions() const
{
	return this->initialDivisions;
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

size_t Path::getSize() const
{
	return this->path.size();
}

Vec3 Path::getIntermediate(float distance) const
{
	float total = 0.0f;
	Vec3 point(
		std::numeric_limits<float>::quiet_NaN(),
		std::numeric_limits<float>::quiet_NaN(),
		std::numeric_limits<float>::quiet_NaN());

	for (size_t i = 0; i < this->path.size() - 1; i++) {
		float length = magnitude(this->path[i + 1] - this->path[i]);
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

size_t Path::getIndex(float distance) const
{
	float total = 0.0f;
	for (size_t i = 0; i < this->path.size(); i++) {
		float length = magnitude(this->path[i + 1] - this->path[i]);
		if (total + length >= distance)
			return i;
		total += length;
	}
	return this->path.size();
}

float Path::getLength() const
{
	return this->length;
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
	float s = 0.0f;
	for (size_t i = 0; i < this->path.size() - 1; i++) {
		s += magnitude(this->path[i+1] - this->path[i]);
		if (s >= t)
			return getDirection(i);
	}
	return getDirection(this->path.size() - 1);
}

float Path::getDistance(size_t index) const
{
	float distance = 0.0f;
	for (size_t i = 0; i < index; i++)
		distance += magnitude(this->path[i + 1] - this->path[i]);
	return distance;
}

float Path::getDistance(size_t start, size_t end) const
{
	float distance = 0.0f;
	for (size_t i = start; i < end; i++)
		distance += magnitude(this->path[i + 1] - this->path[i]);
	return distance;
}

float Path::getSegmentLength(size_t index) const
{
	if (index == 0)
		return 0.0f;
	else
		return magnitude(this->path[index] - this->path[index-1]);
}

size_t Path::toPathIndex(size_t control) const
{
	size_t i = control / this->spline.getDegree();
	if (i == 0)
		return 0;
	return (i-1) * (1+this->divisions) + (1+this->initialDivisions);
}

float Path::getPercentage(size_t index) const
{
	float length = 0.0f;
	for (size_t i = 0; i < index; i++)
		length += magnitude(this->path[i+1] - this->path[i]);
	return length / this->length;
}

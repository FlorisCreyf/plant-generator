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
#include <limits>

void pg::Path::generate()
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

void pg::Path::setSpline(Spline &spline)
{
	this->spline = spline;
	generate();
}

pg::Spline pg::Path::getSpline()
{
	return spline;
}

/** Sets the divisions for each curve in the path. */
void pg::Path::setResolution(int resolution)
{
	this->resolution = resolution;
	generate();
}

int pg::Path::getResolution() const
{
	return resolution;
}

void pg::Path::subdivide(int level)
{
	subdivisions = level;
	// TODO subdivide
}

int pg::Path::getSubdivisions() const
{
	return subdivisions;
}

std::vector<pg::Vec3> pg::Path::get() const
{
	return path;
}

pg::Vec3 pg::Path::get(int index) const
{
	return path[index];
}

int pg::Path::getSize() const
{
	return path.size();
}

pg::Vec3 pg::Path::getIntermediate(float distance) const
{
	Vec3 point = {
		std::numeric_limits<float>::quiet_NaN(),
		std::numeric_limits<float>::quiet_NaN(),
		std::numeric_limits<float>::quiet_NaN()
	};
	float total = 0.0f;

	for (size_t i = 0; i < path.size() - 1; i++) {
		float length = pg::magnitude(path[i + 1] - path[i]);
		if (total + length >= distance) {
			point = (distance - total) * getDirection(i) + path[i];
			break;
		}
		total += length;
	}

	return point;
}

float pg::Path::getLength() const
{
	float length = 0.0f;
	for (size_t i = 0; i < path.size() - 1; i++)
		length += pg::magnitude(path[i + 1] - path[i]);
	return length;
}

pg::Vec3 pg::Path::getDirection(size_t index) const
{
	if (index == path.size() - 1)
		return pg::normalize(path[index] - path[index - 1]);
	else
		return pg::normalize(path[index + 1] - path[index]);
}

pg::Vec3 pg::Path::getIntermediateDirection(float t) const
{
	pg::Vec3 direction;
	float s = 0.0f;
	// t *= getLength();
	for (size_t i = 0; i < path.size() - 1; i++) {
		s += magnitude(path[i+1] - path[i]);
		if (s >= t) {
			direction = getDirection(i);
			break;
		}
	}
	return direction;
}

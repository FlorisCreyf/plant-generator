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

#include "volumetric_path.h"

bool pg::VolumetricPath::operator==(const pg::VolumetricPath &path) const
{
	return Path::operator==(path) &&
		radius == path.radius &&
		minRadius == path.minRadius &&
		maxRadius == path.maxRadius;
}

bool pg::VolumetricPath::operator!=(const pg::VolumetricPath &path) const
{
	return !(*this == path);
}

void pg::VolumetricPath::setMaxRadius(float radius)
{
	maxRadius = radius;
}

float pg::VolumetricPath::getMaxRadius()
{
	return maxRadius;
}

void pg::VolumetricPath::setMinRadius(float radius)
{
	minRadius = radius;
}

float pg::VolumetricPath::getMinRadius()
{
	return minRadius;
}

void pg::VolumetricPath::setRadius(pg::Spline spline)
{
	radius = spline;
}

pg::Spline pg::VolumetricPath::getRadius()
{
	return radius;
}

float pg::VolumetricPath::getRadius(int index)
{
	float length = 0.0f;
	for (int i = 0; i < index; i++)
		length += magnitude(path[i+1] - path[i]);
	float t = length / getLength();
	float z = radius.getPoint(t).z * maxRadius;
	return z < minRadius ? minRadius : z;
}

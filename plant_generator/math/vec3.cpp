/* Copyright 2019 Floris Creyf
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

#include "vec3.h"

using pg::Vec3;

float pg::magnitude(Vec3 vec)
{
	return std::sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
}

Vec3 pg::normalize(Vec3 vec)
{
	Vec3 result;
	float m = magnitude(vec);
	result.x = vec.x / m;
	result.y = vec.y / m;
	result.z = vec.z / m;
	return result;
}

float pg::project(Vec3 a, Vec3 b)
{
	return dot(a, b) / dot(b, b);
}

Vec3 pg::projectOntoPlane(Vec3 vector, Vec3 normal)
{
	return vector - project(vector, normal) * normal;
}

float pg::angle(Vec3 a, Vec3 b)
{
	return acos(dot(a, b) / (magnitude(a) * magnitude(b)));
}

float pg::dot(Vec3 a, Vec3 b)
{
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

Vec3 pg::cross(Vec3 a, Vec3 b)
{
	Vec3 result;
	result.x = a.y*b.z - a.z*b.y;
	result.y = a.z*b.x - a.x*b.z;
	result.z = a.x*b.y - a.y*b.x;
	return result;
}

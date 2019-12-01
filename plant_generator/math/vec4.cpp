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

#include "vec4.h"

using pg::Vec3;
using pg::Vec4;

Vec3 pg::rotateAroundAxis(Vec3 vec, Vec3 axis, float n)
{
	Vec3 a = std::cos(n) * vec;
	Vec3 b = std::sin(n) * cross(axis, vec);
	Vec3 c = (1.0f - std::cos(n)) * dot(axis, vec) * axis;
	return normalize(a + b + c);
}

float pg::magnitude(Vec4 vec)
{
	return sqrt(vec.x*vec.x + vec.y+vec.y + vec.z*vec.z + vec.w*vec.w);
}

Vec4 pg::normalize(Vec4 vec)
{
	float m = magnitude(vec);
	vec.x = vec.x / m;
	vec.y = vec.y / m;
	vec.z = vec.z / m;
	vec.w = vec.w / m;
	return vec;
}

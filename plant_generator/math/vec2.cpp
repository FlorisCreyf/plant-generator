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

#include "vec2.h"
#include <cmath>

using pg::Vec2;

Vec2 pg::perp(Vec2 vec)
{
	return Vec2(-vec.y, vec.x);
}

float pg::magnitude(Vec2 vec)
{
	return std::sqrt(vec.x*vec.x + vec.y*vec.y);
}

Vec2 pg::normalize(Vec2 vec)
{
	Vec2 result;
	float m = magnitude(vec);
	result.x = vec.x / m;
	result.y = vec.y / m;
	return result;
}

float pg::dot(Vec2 a, Vec2 b)
{
	return a.x*b.x + a.y*b.y;
}

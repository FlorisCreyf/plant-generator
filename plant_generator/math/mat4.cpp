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

#include "mat4.h"

using pg::Mat4;
using pg::Vec3;

Vec3 Mat4::apply(const Vec3 vec, float w) const
{
	return toVec3(*this * toVec4(vec, w));
}

Mat4 Mat4::transpose() const
{
	Mat4 result;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			result[i][j] = (*this)[j][i];
	return result;
}

Mat4 pg::rotateIntoVec(const Vec3 &normal, const Vec3 &direction)
{
	Vec3 v = pg::cross(normal, direction);
	float e = pg::dot(normal, direction);
	float h = 1.0f / (1.0f + e);
	return Mat4(
		e + h*v.x*v.x, h*v.x*v.y + v.z, h*v.x*v.z - v.y, 0.0f,
		h*v.x*v.y - v.z, e + h*v.y*v.y, h*v.y*v.z + v.x, 0.0f,
		h*v.x*v.z + v.y, h*v.y*v.z - v.x, e + h*v.z*v.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
}

Mat4 pg::translate(const Vec3 &vec)
{
	return Mat4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		vec.x, vec.y, vec.z, 1.0f);
}

Mat4 pg::scale(const Vec3 &vec)
{
	return Mat4(
		vec.x, 0.0f, 0.0f, 0.0f,
		0.0f, vec.y, 0.0f, 0.0f,
		0.0f, 0.0f, vec.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
}

Mat4 pg::transpose(const Mat4 &mat)
{
	Mat4 result;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			result[j][i] = mat[i][j];
	return result;
}

Mat4 pg::rotateXY(float x, float y)
{
	float sx = std::sin(x);
	float cx = std::cos(x);
	float sy = std::sin(y);
	float cy = std::cos(y);
	return Mat4(
		cy, 0.0f, sy, 0.0f,
		sx*sy, cx, -sx*cy, 0.0f,
		-cx*sy, sx, cx*cy, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
}

Mat4 pg::rotateZYX(float z, float y, float x)
{
	float cz = std::cos(z);
	float sz = std::sin(z);
	float cy = std::cos(y);
	float sy = std::sin(y);
	float cx = std::cos(x);
	float sx = std::sin(x);
	return Mat4(
		cz*cy, sz*cy, -sy, 0.0f,
		cz*sy*sx-sz*sx, sz*sy*sx+cz*cz, cy*sx, 0.0f,
		cz*sy*cx+sz*sx, sz*sy*cx-cz*sx, cy*cx, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
}

Mat4 pg::identity()
{
	return Mat4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
}

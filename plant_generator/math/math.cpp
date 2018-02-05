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

#include "math.h"
#include <cmath>

using pg::Vec2;
using pg::Vec3;
using pg::Mat4;
using pg::Quat;

float pg::abs(float value)
{
	return value < 0 ? -value : value;
}

pg::Vec2 pg::perp(const Vec2 &vec)
{
	Vec2 v = {-vec.y, vec.x};
	return v;
}

float pg::dot(const Vec2 &a, const Vec2 &b)
{
	return a.x*b.x + a.y*b.y;
}

float pg::dot(const Vec3 &a, const Vec3 &b)
{
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

Vec3 pg::cross(const Vec3 &a, const Vec3 &b)
{
	Vec3 v;
	v.x = a.y*b.z - a.z*b.y;
	v.y = a.z*b.x - a.x*b.z;
	v.z = a.x*b.y - a.y*b.x;
	return v;
}

float pg::magnitude(const Vec2 &vec)
{
	return std::sqrt(vec.x*vec.x + vec.y*vec.y);
}

float pg::magnitude(const Vec3 &vec)
{
	return std::sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
}

Vec2 pg::normalize(const Vec2 &vec)
{
	Vec2 v;
	float m = std::sqrt(vec.x*vec.x + vec.y*vec.y);
	v.x = vec.x / m;
	v.y = vec.y / m;
	return v;
}

Vec3 pg::normalize(const Vec3 &vec)
{
	Vec3 v;
	float m = std::sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
	v.x = vec.x / m;
	v.y = vec.y / m;
	v.z = vec.z / m;
	return v;
}

Quat pg::normalize(const Quat &quat)
{
	float n;
	Quat q;
	float x2 = quat.x * quat.x;
	float y2 = quat.y * quat.y;
	float z2 = quat.z * quat.z;
	float w2 = quat.w * quat.w;
	n = std::sqrt(x2 + y2 + z2 + w2);
	q.x = quat.x / n;
	q.y = quat.y / n;
	q.z = quat.z / n;
	q.w = quat.w / n;
	return q;
}

Mat4 pg::transpose(const Mat4 &mat)
{
	Mat4 t;
	int i, j;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			t[i][j] = mat[j][i];
	return t;
}

Mat4 pg::rotateIntoVec(const Vec3 &normal, const Vec3 &direction)
{
	Vec3 v = pg::cross(normal, direction);
	float e = pg::dot(normal, direction);
	float h = 1.0f / (1.0f + e);
	Mat4 m = {
		e + h*v.x*v.x, h*v.x*v.y + v.z, h*v.x*v.z - v.y, 0.0f,
		h*v.x*v.y - v.z, e + h*v.y*v.y, h*v.y*v.z + v.x, 0.0f,
		h*v.x*v.z + v.y, h*v.y*v.z - v.x, e + h*v.z*v.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	return m;
}

Vec3 pg::rotateAroundAxis(const Vec3 &vec, const Vec3 &axis, float n)
{
	Vec3 a = std::cos(n) * vec;
	Vec3 b = std::sin(n) * pg::cross(axis, vec);
	Vec3 c = (1.0f - std::cos(n)) * pg::dot(axis, vec) * axis;
	return pg::normalize(a + b + c);
}

Mat4 pg::translate(const Vec3 &vec)
{
	Mat4 m = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		vec.x, vec.y, vec.z, 1.0f
	};
	return m;
}

Mat4 pg::rotateXY(float x, float y)
{
	float sx = std::sin(x);
	float cx = std::cos(x);
	float sy = std::sin(y);
	float cy = std::cos(y);
	Mat4 m = {
		cy, 0.0f, sy, 0.0f,
		sx*sy, cx, -sx*cy, 0.0f,
		-cx*sy, sx, cx*cy, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	return m;
}

Quat pg::fromAxisAngle(const Vec3 &vec, float theta)
{
	float a = theta / 2.0f;
	float b = std::sin(a);
	Quat q = {vec.x*b, vec.y*b, vec.z*b, std::cos(a)};
	return q;
}

Mat4 pg::quatToMat4(const Quat &quat)
{
	Mat4 m;
	m[0][0] = 1.0f - 2.0f*(quat.y*quat.y + quat.z*quat.z);
	m[1][0] = 2.0f*(quat.x*quat.y - quat.w*quat.z);
	m[2][0] = 2.0f*(quat.x*quat.z + quat.w*quat.y);
	m[0][1] = 2.0f*(quat.x*quat.y + quat.w*quat.z);
	m[1][1] = 1.0f - 2.0f*(quat.x*quat.x + quat.z*quat.z);
	m[2][1] = 2.0f*(quat.y*quat.z - quat.w*quat.x);
	m[0][2] = 2.0f*(quat.x*quat.z - quat.w*quat.y);
	m[1][2] = 2.0f*(quat.y*quat.z + quat.w*quat.x);
	m[2][2] = 1.0f - 2.0f*(quat.x*quat.x + quat.y*quat.y);
	m[3][0] = m[3][1] = m[3][2] = 0.0f;
	m[0][3] = m[1][3] = m[2][3] = 0.0f;
	m[3][3] = 1.0f;
	return m;
}

Quat pg::slerp(const Quat &a, const Quat &b, float t)
{
	float i = std::acos(a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w);
	float j = std::sin(i);
	float x = std::sin(i*(1.0f - t))/j;
	float y = std::sin(i*t)/j;

	Quat m;
	m.x = x*a.x + y*b.x;
	m.y = x*a.y + y*b.y;
	m.z = x*a.z + y*b.z;
	m.w = x*a.w + y*b.w;

	return m;
}

Mat4 pg::identity()
{
	Mat4 m = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	return m;
}

float pg::project(Vec3 &a, Vec3 &b)
{
	return pg::dot(a, b) / pg::dot(b, b);
}

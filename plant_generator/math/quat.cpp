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

#include "quat.h"

using pg::Mat4;
using pg::Quat;
using pg::Vec3;

Mat4 pg::toMat4(Quat quat)
{
	Mat4 result;
	result[0][0] = 1.0f - 2.0f*(quat.y*quat.y + quat.z*quat.z);
	result[1][0] = 2.0f*(quat.x*quat.y - quat.w*quat.z);
	result[2][0] = 2.0f*(quat.x*quat.z + quat.w*quat.y);
	result[0][1] = 2.0f*(quat.x*quat.y + quat.w*quat.z);
	result[1][1] = 1.0f - 2.0f*(quat.x*quat.x + quat.z*quat.z);
	result[2][1] = 2.0f*(quat.y*quat.z - quat.w*quat.x);
	result[0][2] = 2.0f*(quat.x*quat.z - quat.w*quat.y);
	result[1][2] = 2.0f*(quat.y*quat.z + quat.w*quat.x);
	result[2][2] = 1.0f - 2.0f*(quat.x*quat.x + quat.y*quat.y);
	result[3][0] = result[3][1] = result[3][2] = 0.0f;
	result[0][3] = result[1][3] = result[2][3] = 0.0f;
	result[3][3] = 1.0f;
	return result;
}

float pg::norm(Quat quat)
{
	return sqrt(
		quat.x*quat.x +
		quat.y*quat.y +
		quat.z*quat.z +
		quat.w*quat.w);
}

Quat pg::inverse(Quat quat)
{
	float n = norm(quat);
	return conjugate(quat) * (1.0f / (n*n));
}

Quat pg::normalize(Quat quat)
{
	float n = norm(quat);
	quat.x = quat.x / n;
	quat.y = quat.y / n;
	quat.z = quat.z / n;
	quat.w = quat.w / n;
	return quat;
}

Quat pg::fromAxisAngle(Vec3 vec, float theta)
{
	float a = theta / 2.0f;
	float b = std::sin(a);
	return Quat(vec.x*b, vec.y*b, vec.z*b, std::cos(a));
}

Quat pg::nlerp(Quat a, Quat b, float t)
{
	return normalize((1.0f - t) * a + t * b);
}

Quat pg::slerp(Quat a, Quat b, float t)
{
	float theta = a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
	if (theta >= 1.0f)
		return a;

	float i = std::acos(theta);
	float j = std::sin(i);
	float x = std::sin(i*(1.0f - t))/j;
	float y = std::sin(i*t)/j;

	Quat quat;
	quat.x = x*a.x + y*b.x;
	quat.y = x*a.y + y*b.y;
	quat.z = x*a.z + y*b.z;
	quat.w = x*a.w + y*b.w;
	return quat;
}

Quat pg::rotateIntoVecQ(Vec3 normal, Vec3 direction)
{
	float e = sqrt(2.0f * (1.0f + dot(normal, direction)));
	Vec3 vec = (1.0f / e) * cross(normal, direction);
	return Quat(vec.x, vec.y, vec.z, e / 2.0f);
}

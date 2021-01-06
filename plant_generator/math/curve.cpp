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

#include "curve.h"
#include "mat4.h"
#include <cmath>

using pg::Vec3;

int factorial(int n)
{
	int r = 1;
	for (int i = n; i > 0; i--)
		r *= i;
	return r;
}

float getBernsteinTerm(float t, int i, int n)
{
	float a = (float)factorial(n)/(factorial(i) * factorial(n - i));
	float b = pow(t, i) * pow(1.0f - t, n - i);
	return a * b;
}

Vec3 getOtherBezier(float t, const Vec3 *points, int size)
{
	Vec3 b(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < size; i++) {
		float basis = getBernsteinTerm(t, i, size-1);
		b += basis * points[i];
	}
	return b;
}

Vec3 pg::getBezier(float t, const Vec3 *p, int size)
{
	if (size == 4)
		return getCubicBezier(t, p[0], p[1], p[2], p[3]);
	if (size == 2)
		return getLinearBezier(t, p[0], p[1]);
	else
		return getOtherBezier(t, p, size);
}

Vec3 pg::getLinearBezier(float t, Vec3 x, Vec3 y)
{
	Vec3 b(0.0f, 0.0f, 0.0f);
	b += (1.0f-t) * x;
	b += t * y;
	return b;
}

Vec3 pg::getQuadraticBezier(float t, Vec3 x, Vec3 y, Vec3 z)
{
	Vec3 b(0.0f, 0.0f, 0.0f);
	float s = 1.0f-t;
	b += (s*s) * x;
	b += (2.0f*t*s) * y;
	b += (t*t) * z;
	return b;
}

Vec3 pg::getCubicBezier(float t, Vec3 x, Vec3 y, Vec3 z, Vec3 w)
{
	Vec3 b(0.0f, 0.0f, 0.0f);
	float s = 1.0f-t;
	b += (s*s*s) * x;
	b += (3.0f*t*s*s) * y;
	b += (3.0f*t*t*s) * z;
	b += (t*t*t) * w;
	return b;
}

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
using pg::Mat4;

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

Vec3 pg::getBezier(float t, const Vec3 *points, int size)
{
	Vec3 b(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < size; i++) {
		float basis = getBernsteinTerm(t, i, size-1);
		Vec3 a = basis * points[i];
		b = a + b;
	}

	return b;
}

Vec3 pg::getBezierPath(float t, const Vec3 *points, int size)
{
	int curves = (size - 1) / 3;
	int curve = 0;

	for (int i = 3; curve < curves; i += 3, curve++)
		if (points[i].x > t) {
			t -= points[i-3].x;
			t *= 1.0f/(points[i].x - points[i-3].x);
			break;
		}

	if (curve == curves)
		curve = curves - 1;
	if (t > 1.0f)
		t = 1.0f;

	return getBezier(t, &points[curve*4], 4);
}

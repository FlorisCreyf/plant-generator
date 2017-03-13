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
#include <cmath>

using namespace treemaker;

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

Vec3 treemaker::getBezier(float t, Vec3 *points, size_t size)
{
	Vec3 b = {0.0f, 0.0f, 0.0f};

	for (unsigned i = 0; i < size; i++) {
		float basis = getBernsteinTerm(t, i, size-1);
		Vec3 a = basis * points[i];
		b = a + b;
	}

	return b;
}

Vec3 treemaker::getBezierPath(float t, Vec3 *points, size_t size)
{
	size_t curves = size / 4;
	size_t curve = 0;

	for (size_t i = 3; curve < curves; i += 4, curve++)
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

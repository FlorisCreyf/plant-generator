/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "curve.h"
#include <math.h>

int factorial(int n)
{
	int r = 1;
	int i;
	for (i = n; i > 0; i--)
		r *= i;
	return r;
}

float getBernsteinTerm(float t, int i, int n)
{
	float a = (float)factorial(n)/(factorial(i) * factorial(n - i));
	float b = pow(t, i) * pow(1.f - t, n - i);
	return a * b;
}

TMvec3 tmGetBezier(float t, TMvec3 *p, int degree)
{
	TMvec3 a;
	TMvec3 b = {0.f, 0.f, 0.f};
	float basis;
	int i;

	for (i = 0; i < degree; i++) {
		basis = getBernsteinTerm(t, i, degree-1);
		a = tmMultVec3(basis, &p[i]);
		b = tmAddVec3(&a, &b);
	}

	return b;
}

TMvec3 tmGetPath(float t, TMvec3 *p, int curveCount)
{
	int curve;
	int i;

	for (i = 3, curve = 0; curve < curveCount; i += 4, curve++)
		if (p[i].x > t) {
			t -= p[i-3].x;
			t *= 1.0f/(p[i].x - p[i-3].x);
			break;
		}

	if (curve == curveCount)
		curve = curveCount - 1;
	if (t > 1.0f)
		t = 1.0f;

	return tmGetBezier(t, &p[curve*4], 4);
}

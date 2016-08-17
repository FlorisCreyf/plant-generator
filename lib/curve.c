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

float get_bernstein_term(float t, int i, int n)
{
	float a = (float)factorial(n)/(factorial(i) * factorial(n - i));
	float b = pow(t, i) * pow(1.f - t, n - i);
	return a * b;
}

tm_vec3 tm_get_bezier(float t, tm_vec3 *p, int degree)
{
	tm_vec3 a;
	tm_vec3 b = {0.f, 0.f, 0.f};
	float basis;
	int i = 0;

	for (; i < degree; i++) {
		basis = get_bernstein_term(t, i, degree-1);
		a = tm_mult_vec3(basis, &p[i]);
		b = tm_add_vec3(&a, &b);
	}

	return b;
}

tm_vec3 tm_get_path(float t, tm_vec3 *p, int curve_count)
{
	int i;
	int curve;

	for (i = 3, curve = 0; curve < curve_count; i += 4, curve++)
		if (p[i].x > t) {
			t -= p[i-3].x;
			t *= 1.0f/(p[i].x - p[i-3].x);
			break;
		}

	if (curve == curve_count)
		curve = curve_count - 1;
	if (t > 1.0f)
		t = 1.0f;

	return tm_get_bezier(t, &p[curve*4], 4);
}

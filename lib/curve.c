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

vec3 bt_get_bezier(float t, vec3 *p, int degree)
{
	vec3 a;
	vec3 b = {0.f, 0.f, 0.f};
	float basis;
	int i = 0;

	for (; i < degree; i++) {
		basis = get_bernstein_term(t, i, degree-1);
		a = bt_mult_vec3(basis, &p[i]);
		b = bt_add_vec3(&a, &b);
	}

	return b;
}

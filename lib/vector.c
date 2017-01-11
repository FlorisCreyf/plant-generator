/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "vector.h"
#include <math.h>

float absf(float f)
{
	return f < 0 ? -f : f;
}

float tmDotVec3(TMvec3 *a, TMvec3 *b)
{
	return a->x*b->x + a->y*b->y + a->z*b->z;
}

TMvec3 tmCrossVec3(TMvec3 *a, TMvec3 *b)
{
	float x = a->y*b->z - a->z*b->y;
	float y = a->z*b->x - a->x*b->z;
	float z = a->x*b->y - a->y*b->x;
	return (TMvec3){x, y, z};
}

TMvec3 tmMultVec3(float a, TMvec3 *b)
{
	TMvec3 v;
	v.x = b->x * a;
	v.y = b->y * a;
	v.z = b->z * a;
	return v;
}

TMvec3 tmAddVec3(TMvec3 *a, TMvec3 *b)
{
	float x = a->x + b->x;
	float y = a->y + b->y;
	float z = a->z + b->z;
	return (TMvec3){x, y, z};
}

TMvec3 tmSubVec3(TMvec3 *a, TMvec3 *b)
{
	float x = a->x - b->x;
	float y = a->y - b->y;
	float z = a->z - b->z;
	return (TMvec3){x, y, z};
}

TMmat4 tmMultMat4(TMmat4 *a, TMmat4 *b)
{
	int row = 0;
	int col = 0;
	TMmat4 m;

	for (row = 0; row < 4; row++)
		for (col = 0; col < 4; col++)
			m.m[row][col] =
				a->m[0][col]*b->m[row][0] +
				a->m[1][col]*b->m[row][1] +
				a->m[2][col]*b->m[row][2] +
				a->m[3][col]*b->m[row][3];

	return m;
}

void tmNormalizeVec3(TMvec3 *a)
{
	float m = sqrt(a->x*a->x + a->y*a->y + a->z*a->z);
	a->x /= m;
	a->y /= m;
	a->z /= m;
}

TMmat4 tmTransposeMat4(TMmat4 *m)
{
	TMmat4 t;
	int i, j;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			t.m[i][j] = m->m[j][i];
	return t;
}

float tmMagVec3(TMvec3 *a)
{
	return sqrt(a->x*a->x + a->y*a->y + a->z*a->z);
}

TMmat4 tmRotateIntoVec(TMvec3 *normal, TMvec3 *direction)
{
	TMvec3 v = tmCrossVec3(normal, direction);
	float e = tmDotVec3(normal, direction);
	float h = 1 / (1 + e);
	return (TMmat4){
		e + h*v.x*v.x, h*v.x*v.y + v.z, h*v.x*v.z - v.y, 0.0f,
		h*v.x*v.y - v.z, e + h*v.y*v.y, h*v.y*v.z + v.x, 0.0f,
		h*v.x*v.z + v.y, h*v.y*v.z - v.x, e + h*v.z*v.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
}

TMvec3 tmRotateAroundAxis(TMvec3 *v, TMvec3 *axis, float n)
{
	TMvec3 a, b, c, d;
	a = tmMultVec3(cos(n), v);
	b = tmCrossVec3(axis, v);
	b = tmMultVec3(sin(n), &b);
	c = tmMultVec3((1.f - cos(n)) * tmDotVec3(axis, v), axis);
	d = tmAddVec3(&b, &c);
	d = tmAddVec3(&a, &d);
	tmNormalizeVec3(&d);
	return d;
}

TMmat4 tmTranslate(float x, float y, float z)
{
	return (TMmat4){
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		x, y, z, 1.0f
	};
}

TMmat4 tmRotateXY(float x, float y)
{
	float sx = sin(x);
	float cx = cos(x);
	float sy = sin(y);
	float cy = cos(y);
	return (TMmat4){
		cy, 0.0f, sy, 0.0f,
		sx*sy, cx, -sx*cy, 0.0f,
		-cx*sy, sx, cx*cy, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
}

float tmTransform(TMvec3 *v, TMmat4 *t, float w)
{
	float x, y, z, h;
	x = v->x*t->m[0][0] + v->y*t->m[1][0] + v->z*t->m[2][0] + t->m[3][0]*w;
	y = v->x*t->m[0][1] + v->y*t->m[1][1] + v->z*t->m[2][1] + t->m[3][1]*w;
	z = v->x*t->m[0][2] + v->y*t->m[1][2] + v->z*t->m[2][2] + t->m[3][2]*w;
	h = v->x*t->m[0][3] + v->y*t->m[1][3] + v->z*t->m[2][3] + t->m[3][3]*w;
	v->x = x;
	v->y = y;
	v->z = z;
	return h;
}

TMquat tmFromAxisAngle(float x, float y, float z, float theta)
{
	float a = theta / 2.0f;
	float b = sin(a);
	return (TMquat){x*b, y*b, z*b, cos(a)};
}

TMmat4 tmQuatToMat4(TMquat *q)
{
	TMmat4 m;
	m.m[0][0] = 1.0f - 2.0f*(q->y*q->y + q->z*q->z);
	m.m[1][0] = 2.0f*(q->x*q->y - q->w*q->z);
	m.m[2][0] = 2.0f*(q->x*q->z + q->w*q->y);
	m.m[0][1] = 2.0f*(q->x*q->y + q->w*q->z);
	m.m[1][1] = 1.0f - 2.0f*(q->x*q->x + q->z*q->z);
	m.m[2][1] = 2.0f*(q->y*q->z - q->w*q->x);
	m.m[0][2] = 2.0f*(q->x*q->z - q->w*q->y);
	m.m[1][2] = 2.0f*(q->y*q->z + q->w*q->x);
	m.m[2][2] = 1.0f - 2.0f*(q->x*q->x + q->y*q->y);
	m.m[3][0] = m.m[3][1] = m.m[3][2] = 0.0f;
	m.m[0][3] = m.m[1][3] = m.m[2][3] = 0.0f;
	m.m[3][3] = 1.0f;
	return m;
}

TMquat tmMultQuat(TMquat *a, TMquat *b)
{
	float s = a->w*b->w - tmDotVec3(&(a->v), &(b->v));
	TMvec3 x = tmCrossVec3(&(a->v), &(b->v));
	TMvec3 y = tmMultVec3(b->w, &(a->v));
	TMvec3 z = tmMultVec3(a->w, &(b->v));
	TMvec3 f = tmAddVec3(&x, &y);
	f = tmAddVec3(&f, &z);
	return (TMquat){f.x, f.y, f.z, s};
}

void tmNormalizeQuat(TMquat *q)
{
	float n = sqrt(q->x*q->x + q->y*q->y + q->z*q->z + q->w*q->w);
	q->x /= n;
	q->y /= n;
	q->z /= n;
	q->w /= n;
}

TMquat tmSlerp(TMquat *a, TMquat *b, float t)
{
	float i = acos(a->x*b->x + a->y*b->y + a->z*b->z + a->w*b->w);
	float j = sin(i);
	float x = sin(i*(1.0f-t))/j;
	float y = sin(i*t)/j;

	float dot = a->x*b->x + a->y*b->y + a->z*b->z + a->w+b->w;

	TMquat m;
	m.x = x*a->x + y*(b->x);
	m.y = x*a->y + y*(b->y);
	m.z = x*a->z + y*(b->z);
	m.w = x*a->w + y*(b->w);

	return m;
}

TMmat4 tmMat4Identity()
{
	return (TMmat4){
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
}

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

typedef bt_vec3 vec3;
typedef bt_mat4 mat4;

int bt_dot_vec3(vec3 *a, vec3 *b)
{
	return a->x*b->x + a->y*b->y + a->z*b->z;
}

vec3 bt_cross_vec3(vec3 *a, vec3 *b)
{
	float x = a->y*b->z - a->z*b->y;
	float y = a->z*b->x - a->x*b->z;
	float z = a->x*b->y - a->y*b->x;

	return (vec3){x, y, z};
}

vec3 bt_add_vec3(vec3 *a, vec3 *b) 
{
	float x = a->x + b->x;
	float y = a->y + b->y;
	float z = a->z + b->z;

	return (vec3){x, y, z};
}

vec3 bt_sub_vec3(vec3 *a, vec3 *b)
{
	float x = a->x - b->x;
	float y = a->y - b->y;
	float z = a->z - b->z;

	return (vec3){x, y, z};	
}

mat4 bt_mult_mat4(mat4 *a, mat4 *b)
{
	int row = 0;
	int col = 0;
	mat4 m;

	for (col = 0; col < 4; col++)
		for (row = 0; row < 4; row++)	
			m.m[row][col] =
				a->m[col][row]*b->m[row][col] +
				a->m[col][row]*b->m[row][col] +
				a->m[col][row]*b->m[row][col] +
				a->m[col][row]*a->m[row][col];

	return m;
}

void bt_normalize_vec3(vec3 *a)
{
	float m = sqrt(a->x*a->x + a->y*a->y + a->z*a->z);
	a->x /= m;
	a->y /= m;
	a->z /= m;
}

mat4 bt_rotate_into_vec(vec3 *normal, vec3 *direction)
{
	vec3 v = bt_cross_vec3(normal, direction);
	float e = bt_dot_vec3(normal, direction);
	float h = 1 / (1 + e);
	
	return (mat4){
		e + h*v.x*v.x, h*v.x*v.y + v.z, h*v.x*v.z - v.y, 0.0f,
		h*v.x*v.y - v.z, e + h*v.y*v.y, h*v.y*v.z + v.x, 0.0f,
		h*v.x*v.z + v.y, h*v.y*v.z - v.x, e + h*v.z*v.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
}

void bt_add_translation(mat4 *m, vec3 *translation)
{
	m->m[3][0] = translation->x;
	m->m[3][1] = translation->y;
	m->m[3][2] = translation->z;
}

void bt_point_transform(vec3 *v, mat4 *t)
{
	float x, y, z;
	x = v->x*t->m[0][0] + v->y*t->m[1][0] + v->z*t->m[2][0] + t->m[3][0];
	y = v->x*t->m[0][1] + v->y*t->m[1][1] + v->z*t->m[2][1] + t->m[3][1];
	z = v->x*t->m[0][2] + v->z*t->m[1][2] + v->z*t->m[2][2] + t->m[3][2];

	v->x = x;
	v->y = y;
	v->z = z;
}


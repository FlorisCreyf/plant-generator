/* 
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "vector.h"

typedef bt_vec3 vec3;
typedef bt_mat4 mat4;

int bt_dotvec3(vec3 *a, vec3 *b)
{
	return a->x*b->x + a->y*b->y + a->z*b->z;
}

vec3 bt_crossvec3(vec3 *a, vec3 *b)
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

mat4 bt_rotate_into_vec(vec3 *normal, vec3 *direction)
{
	vec3 v = bt_crossvec3(normal, direction);
	float e = bt_dotvec3(normal, direction);
	float h = 1 / (1 + e);

	return (mat4){
		e + h*v.x*v.x, h*v.x*v.y - v.z, h*v.x*v.z + v.y, 0.0f,
		h*v.x*v.y + v.z, e + h*v.y*v.y, h*v.y*v.z - v.x, 0.0f,
		h*v.x*v.z - v.y, h*v.y*v.z + v.x, e + h*v.z*v.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f};
}

void bt_add_translation(mat4 *m, vec3 *translation)
{
	m->x4 = translation->x;
	m->y4 = translation->y;
	m->z4 = translation->z;
}

void bt_point_transform(vec3 *v, mat4 *t)
{
	float x = v->x*t->x1 + v->y*t->x2 + v->z*t->x3 + t->x4;
	float y = v->x*t->y1 + v->y*t->y2 + v->z*t->y3 + t->y4;
	float z = v->x*t->z1 + v->z*t->z2 + v->z*t->z3 + t->z4;
	
	v->x = x;
	v->y = y;
	v->z = z;
}

mat4 bt_mult_mat4(mat4 *a, mat4 *b)
{
	bt_mat4 m;

	return m;
}


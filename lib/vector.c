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
	return a->x*b->x + a->x*a->y;
}

vec3 bt_crossvec3(vec3 *a, vec3 *b)
{
	return (vec3) {0.0f, 0.0f, 0.0f};
}

void bt_rotate_into_vec(vec3 *a, vec3 *b)
{
	
}

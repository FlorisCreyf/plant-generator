/* 
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "model_builder.h"
#include "node.h"
#include "vector.h"
#include <math.h>

typedef bt_vec3 vec3;
typedef bt_mat4 mat4;

void build_model(float *vertexBuffer, unsigned short elementBuffer)
{

}

void makeCrossSection(float *buffer, node *stem, vec3 *position)
{
	vec3 point;
	vec3 normal = (vec3){0.0f, 1.0f, 0.0f};
	mat4 t = bt_rotate_into_vec(&normal, &(stem->direction));
	int i = 0;
	const int SIZE = stem->branch_resolution * 3;
	const float ROTATION = 360.0f / stem->branch_resolution * M_PI / 180.0f;
	float angle = 0.0f;

	while (i < SIZE) {
		point.x = cosf(angle) * stem->radius;
		point.y = sinf(angle) * stem->radius;
		point.y = 0.0f;
				
		buffer[i++] = point.x;
		buffer[i++] = point.y;
		buffer[i++] = point.z;
		
		bt_multiply_transform(&point, &t);

		angle += ROTATION;
	}
}

void addElementIndices()
{

}


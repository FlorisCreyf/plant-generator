/* 
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "model_builder.h"
#include "mesh_size.h"
#include "vector.h"
#include <math.h>
#include <stdlib.h>

typedef bt_vec3 vec3;
typedef bt_mat4 mat4;

static float *vbo;
static unsigned short *ebo;
static int vbo_size;
static int ebo_size;
static unsigned short vbo_index;
static unsigned short ebo_index;

void make_cross_section(float *buffer, node *stem)
{
	vec3 point;
	vec3 normal = (vec3){0.0f, 1.0f, 0.0f};
	mat4 t = bt_quat_to_mat4(&(stem->direction));
	int i = 0;
	const int SIZE = stem->branch_resolution * 3;
	const float TO_RAD = M_PI / 180.0f;
	const float ROTATION = 360.0f / stem->branch_resolution * TO_RAD;
	float angle = 0.0f;

	while (i < SIZE) {
		point.x = cosf(angle) * stem->radius;
		point.z = sinf(angle) * stem->radius;
		point.y = 0.0f;
		
		buffer[i++] = point.x;
		buffer[i++] = point.y;
		buffer[i++] = point.z;
		
		bt_point_transform(&point, &t);

		angle += ROTATION;
	}
}

void add_element_indices()
{

}

unsigned short add_segment(node *stem, vec3 position, node *parent)
{
	unsigned short l_index;
	unsigned short r_index;
	
	if (stem == NULL)
		return 0;
	
	make_cross_section(&vbo[vbo_index], stem);
	vbo_index += get_branch_vcount(stem);	

	//l_index = add_segment(stem->left, position, stem);
	//r_index = add_segment(stem->right, position, stem);
	
	return vbo_index;
}

void build_model(float *vb, int vb_size, unsigned short *eb, int eb_size,
		node *root)
{
	vec3 origin = (vec3){0.0f, 0.0f, 0.0f};

	vbo = vb;	
	ebo = eb;
	vbo_size = vb_size;
	ebo_size = eb_size;
	vbo_index = 0;
	ebo_index = 0;

	add_segment(root, origin, NULL);
}

int get_vbo_size()
{
	return vbo_index;
}


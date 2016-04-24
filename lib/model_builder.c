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

void make_cross_section(float *buffer, bt_mat4 *t, float radius, float res)
{
	vec3 point;
	const int SIZE = res * 3;
	const float ROTATION = 360.0f / res * M_PI / 180.0f;
	float angle = 0.0f;
	int i = 0;

	while (i < SIZE) {
		point.x = cosf(angle) * radius;
		point.z = sinf(angle) * radius;
		point.y = 0.0f;
		
		bt_point_transform(&point, t);

		buffer[i++] = point.x;
		buffer[i++] = point.y;
		buffer[i++] = point.z;

		angle += ROTATION;
	}
}

void add_element_indices()
{

}

bt_mat4 get_branch_transform(node *stem, float offset)
{
	bt_mat4 rotation = bt_quat_to_mat4(&(stem->direction));
	bt_mat4 translation;
	bt_vec3 pos = stem->position;
	bt_vec3 seg_pos = (bt_vec3){0.0f, offset, 0.0f};
	pos = bt_add_vec3(&pos, &seg_pos);
	translation = bt_translate(pos.x, pos.y, pos.z);

	return bt_mult_mat4(&translation, &rotation);
}

float scale_branch(node *stem, float i)
{
	float c = stem->cross_sections;
	return (c - i) / c;
}

unsigned short add_branch(node *stem, node *parent)
{
	unsigned short l_index;
	unsigned short r_index;
	float offset;
	float percent;
	float radius;
	bt_mat4 transform;
	int i;
	
	if (stem == NULL)
		return 0;

	for (i = 0; i < stem->cross_sections; i++) {
		offset = stem->branch_length / stem->cross_sections * i;
		radius = stem->radius * scale_branch(stem, i);
		transform = get_branch_transform(stem, offset);
		make_cross_section(&vbo[vbo_index], &transform,
				radius, stem->branch_resolution);
		vbo_index += get_branch_vcount(stem);	
	}

	l_index = add_branch(stem->left, stem);
	r_index = add_branch(stem->right, stem);

	return vbo_index;
}

void build_model(float *vb, int vb_size, unsigned short *eb, int eb_size,
		node *root)
{
	vbo = vb;	
	ebo = eb;
	vbo_size = vb_size;
	ebo_size = eb_size;
	vbo_index = 0;
	ebo_index = 0;

	add_branch(root, NULL);
}

int get_vbo_size()
{
	return vbo_index / 3;
}


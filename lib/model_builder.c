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
#include <time.h>
#include <stdlib.h>

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

void add_element_indices(unsigned short *buffer, int l, int h, int res)
{
	const int SIZE = (res - 1) * 6;
	const int INIT_H = h;
	const int INIT_L = l;
	int i = 0;

	while (i < SIZE) {
		buffer[i++] = h;
		buffer[i++] = ++h;
		buffer[i++] = l;

		buffer[i++] = l;
		buffer[i++] = ++l;
		buffer[i++] = h;
	}

	/* Connect last vertices with first. */
	buffer[i++] = l;
	buffer[i++] = INIT_L;
	buffer[i++] = INIT_H;

	buffer[i++] = INIT_H;
	buffer[i++] = h;
	buffer[i++] = l;
}


mat4 get_branch_transform(node *stem, float offset)
{
	float r1, r2;
	mat4 rotation = bt_quat_to_mat4(&(stem->direction));
	mat4 translation;
	vec3 pos = stem->position;
	vec3 seg_pos = (vec3){0.0f, offset, 0.0f};
	pos = bt_add_vec3(&pos, &seg_pos);
	
	/* Add noise because we do not want a perfect cylinder. */
	r1 = ((float)rand() / RAND_MAX - 0.5f) * 0.1f;
	r2 = ((float)rand() / RAND_MAX - 0.5f) * 0.1f;

	translation = bt_translate(pos.x + r1, pos.y + r1, pos.z + r2);

	return bt_mult_mat4(&translation, &rotation);
}

float scale_branch(node *stem, float i)
{
	float c = stem->cross_sections;
	float s = (c - i) / c;
	return pow(s, 0.75f);
}

unsigned short add_branch(node *stem, node *parent)
{
	unsigned short l_index;
	unsigned short r_index;
	float prev_index;
	float offset;
	float radius;
	mat4 transform;
	int i;
	
	if (stem == NULL)
		return 0;

	for (i = 0; i < stem->cross_sections; i++) {
		offset = stem->branch_length / stem->cross_sections * i;
		radius = stem->radius * scale_branch(stem, i);
		transform = get_branch_transform(stem, offset);
		make_cross_section(&vbo[vbo_index], &transform,
				radius, stem->branch_resolution);

		prev_index = vbo_index;
		vbo_index += get_branch_vcount(stem);

		add_element_indices(&ebo[ebo_index], prev_index / 3, 
				vbo_index / 3, stem->branch_resolution);

		ebo_index += get_branch_ecount(stem);
	}

	l_index = add_branch(stem->left, stem);
	r_index = add_branch(stem->right, stem);

	return vbo_index;
}

void build_model(float *vb, int vb_size, unsigned short *eb, int eb_size,
		node *root)
{
	srand(time(0));
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

int get_ebo_size()
{
	return ebo_index;
}


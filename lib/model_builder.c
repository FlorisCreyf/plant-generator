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
#include <stdio.h>
#include <stdlib.h>

static float *vbo;
static unsigned short *ebo;
static int vbo_size;
static int ebo_size;
static unsigned int vbo_index;
static unsigned short ebo_index;

void add_cs_point(float *buffer, float radius, float angle, bt_mat4 *t, int *i)
{
	vec3 normal = {cosf(angle), 0.0f, sinf(angle)};
	vec3 point = bt_mult_vec3(radius, &normal);

	bt_transform(&normal, t, 0.0f);
	bt_transform(&point, t, 1.0f);

	buffer[(*i)++] = point.x;
	buffer[(*i)++] = point.y;
	buffer[(*i)++] = point.z;
	buffer[(*i)++] = normal.x;
	buffer[(*i)++] = normal.y;
	buffer[(*i)++] = normal.z;
}

void make_cross_section(float *buffer, bt_mat4 *t, float radius, float res)
{
	const float ROTATION = 360.0f / res * M_PI / 180.0f;
	float angle = 0.0f;
	int i = 0;

	while (i < res*6) {
		add_cs_point(buffer, radius, angle, t, &i);
		angle += ROTATION;
	}
}

int add_rect(unsigned short *buffer, int *l, int *h, int *i)
{
	buffer[(*i)++] = *h;
	buffer[(*i)++] = ++(*h);
	buffer[(*i)++] = *l;

	buffer[(*i)++] = *l;
	buffer[(*i)++] = *h;
	buffer[(*i)++] = ++(*l);
}

int add_last_rect(unsigned short *buffer, int l, int h, int il, int ih, int *i)
{
	buffer[(*i)++] = ih;
	buffer[(*i)++] = il;
	buffer[(*i)++] = l;

	buffer[(*i)++] = l;
	buffer[(*i)++] = h;
	buffer[(*i)++] = ih;
}

int add_triangle_ring(unsigned short *buffer, int l, int h, int res)
{
	int init_h = h;
	int init_l = l;
	int i = 0;
	while (i < 6*(res-1))
		add_rect(buffer, &l, &h, &i);
	add_last_rect(buffer, l, h, init_l, init_h, &i);
	return i;
}

int add_rect_seg(unsigned short *buffer, int l, int h, int s, int e, int res)
{
	int init_h = h;
	int init_l = l;
	int i = 0, j = 0;

	for (; j < res-1; j++)
		if (j >= s && j <= e)
			add_rect(buffer, &l, &h, &i);
		else {
			l++;
			h++;
		}

	if (j >= s && j <= e) {
		l = init_l + res - 1;
		h = init_h + res - 1;
		add_last_rect(buffer, l, h, init_l, init_h, &i);
	}

	return i;
}

mat4 get_branch_transform(struct line_t *l, float offset)
{
	float r1;
	float r2;
	vec3 normal = {0.0f, 1.0f, 0.0f};
	mat4 rotation = bt_rotate_into_vec(&normal, &(l->direction));
	mat4 translation;
	mat4 transform;

	translation = bt_translate(0.0f, offset, 0.0f);
	transform = bt_translate(
			l->start.x,
			l->start.y,
			l->start.z);
	transform = bt_mult_mat4(&transform, &rotation);
	return bt_mult_mat4(&transform, &translation);
}

float get_radius(node *stem, float c)
{
	c = stem->radius * pow(1.0f - c, 0.75f);
	return c > stem->min_radius ? c : stem->min_radius;
}

void add_cross_section(float *buffer, node *stem, float percent)
{
	float offset;
	int j = get_line(stem, percent, &offset);
	mat4 t = get_branch_transform(&stem->lines[j], offset);
	float r = get_radius(stem, percent);
	make_cross_section(buffer, &t, r, stem->resolution);
}

void add_cross_section_o(float *buffer, node *stem, line *l, float offset)
{
	float len = get_line_length(stem);
	float p = (len - offset + l->length)/len;
	mat4 t = get_branch_transform(l, offset);
	float r = get_radius(stem, p);
	make_cross_section(buffer, &t, r, stem->resolution);
}

int get_max_side(float *l, float *u, int size)
{
	const int ATTRIBS = 2;
	int i = 0, j = 0;
	float max_len = 0.0f;
	int max;

	for (; i < size*3*ATTRIBS; i += 3*ATTRIBS, j++) {
		float a = pow(l[i+0] - u[i+0], 2);
		float b = pow(l[i+1] - u[i+1], 2);
		float c = pow(l[i+2] - u[i+2], 2);
		float len = sqrt(a + b + c);

		if (len > max_len) {
			max_len = len;
			max = j;
		}
	}

	return max;
}

void add_branch(node *stem, node *parent, float offset);

void connect_dichotomous(int a, int b, int c, int t)
{
	int max = get_max_side(&vbo[a], &vbo[b], t);
	int ub = max+2;
	int lb = max-3;
	int start = ub+1 > t ? ub+1-t : 0;
	int end = lb-1 < 0 ? t+lb-1 : t;
	int x, y, z, i;

	a /= 6;
	b /= 6;
	c /= 6;

	/* connect trunk to stem leaving an opening */
	ebo_index += add_rect_seg(&ebo[ebo_index], a, b, start, lb-1, t);
	ebo_index += add_rect_seg(&ebo[ebo_index], a, b, ub+1, end, t);

	/* connect trunk to second stem within opening */
	ebo_index += add_rect_seg(&ebo[ebo_index], a, c, lb, ub, t);
	if (ub >= t)
		ebo_index += add_rect_seg(&ebo[ebo_index], a, c, 0, ub-t, t);
	else if (lb < 0)
		ebo_index += add_rect_seg(&ebo[ebo_index], a, c, t+lb, t, t);

	/* connect remainder */
	z = ub+1 >= t ? ub+1-t : ub+1;
	ebo[ebo_index++] = a + z;
	ebo[ebo_index++] = b + z;
	ebo[ebo_index++] = c + z;

	for (i = 1; i < 7; i++) {
		z = ub+i >= t ? ub+i-t : ub+i;
		x = (i-1)*2;
		y = z-x >= 0 ? z-x : t+(z-x);
		ebo[ebo_index++] = b + y;
		ebo[ebo_index++] = c + z;
		ebo[ebo_index++] = c + (z+1 >= t ? 0 : z+1);
	}

	z = lb < 0 ? t+lb : lb;
	ebo[ebo_index++] = a + z;
	ebo[ebo_index++] = b + z;
	ebo[ebo_index++] = c + z;

	for (i = 1; i < 7; i++) {
		z = lb-i+1 < 0 ? t+lb-i+1 : lb-i+1;
		x = (i-1)*2;
		y = z+x < t ? z+x : z+x-t;
		ebo[ebo_index++] = b + y;
		ebo[ebo_index++] = b + (y+1 >= t ? 0 : y+1);
		ebo[ebo_index++] = c + z;
	}
}

void add_dichotomous(node *n, node *p, int prev_index)
{
	struct line_t *l = &p->lines[p->line_count-1];
	float offset = l->length - n->radius;
	int init_index = vbo_index;

	add_cross_section_o(&vbo[prev_index], p, l, offset);
	add_branch(&n[0], p, 0.05);
	connect_dichotomous(prev_index, init_index, vbo_index, p->resolution);
	add_branch(&n[1], p, 0.05);
}

int add_subbranches(node *stem, int prev_index)
{
	int i = 0;
	int dichotomous = 0;
	vec3 n;
	vec3 p = get_line_end_point(&(stem->lines[stem->line_count-1]));

	for (; i < stem->branch_count; i++) {
		n = stem->branches[i].lines[0].start;
		if (p.x == n.x && p.y == n.y && p.z == n.z) {
			add_dichotomous(&stem->branches[i], stem, prev_index);
			dichotomous++;
			i++;
		} else
			add_branch(&stem->branches[i], stem, 0.0f);
	}

	return dichotomous == 0 ? 0 : 1;
}

void cap_branch(int vertex, int t)
{
	int i;
	for (i = 0; i < t/2-1; i++) {
		ebo[ebo_index++] = vertex + i;
		ebo[ebo_index++] = vertex + i + 1;
		ebo[ebo_index++] = vertex + t - i - 1;

		ebo[ebo_index++] = vertex + i + 1;
		ebo[ebo_index++] = vertex + t - i - 2;
		ebo[ebo_index++] = vertex + t - i - 1;
	}
}

float offset_to_percent(node *stem, float offset, int i)
{
	float len = get_line_length(stem);
	float a = (len - offset) / (stem->cross_sections-1);
	return (offset + i*a) / len;
}

void add_branch(node *stem, node *parent, float offset)
{
	unsigned short l_index;
	unsigned short r_index;
	int prev_index;
	int i, j;
	float p;

	if (stem == NULL)
		return;

	for (i = 0; i < stem->cross_sections - 1; i++) {
		p = offset_to_percent(stem, offset, i);
		add_cross_section(&vbo[vbo_index], stem, p);
		prev_index = vbo_index;
		vbo_index += get_branch_vcount(stem);
		add_triangle_ring(&ebo[ebo_index], prev_index/6,
				vbo_index/6, stem->resolution);
		ebo_index += get_branch_ecount(stem);
	}

	prev_index = vbo_index;
	vbo_index += get_branch_vcount(stem);
	if (add_subbranches(stem, prev_index) == 0) {
		add_cross_section(&vbo[prev_index], stem, 1.0f);
		cap_branch(prev_index/6, stem->resolution);
	}
}

void build_model(float *vb, int vb_size, unsigned short *eb, int eb_size,
		node *root)
{
	int i;
	vbo = vb;
	ebo = eb;
	vbo_size = vb_size;
	ebo_size = eb_size;
	vbo_index = 0;
	ebo_index = 0;

	add_branch(root, NULL, 0.0f);
}

int get_vbo_size()
{
	return vbo_index/6;
}

int get_ebo_size()
{
	return ebo_index;
}

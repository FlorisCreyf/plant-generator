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
static int overflow;

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

vec3 get_cs_normal(int i)
{
	vec3 a = {vbo[i+3], vbo[i+4], vbo[i+5]};
	vec3 b = {vbo[i+9], vbo[i+10], vbo[i+11]};
	vec3 c = bt_cross_vec3(&b, &a);
	bt_normalize_vec3(&c);
	return c;
}

float get_branch_angle(int a, vec3 dir)
{
	vec3 norm_a = get_cs_normal(a);
	vec3 y = {0.f, 1.f, 0.f};
	mat4 rot = bt_rotate_into_vec(&norm_a, &y);
	bt_transform(&dir, &rot, 0.0f);
	float angle = atan(dir.z/dir.x);
	if (dir.z < 0 && dir.x >= 0)
		angle += 2.f*M_PI;
	else if (dir.z < 0 && dir.x < 0)
		angle += M_PI;
	else if (dir.z >= 0 && dir.x < 0)
		angle += M_PI;
	return angle;
}

void get_bounds(vec3 dir, int t, int a, int *lb, int *ub, float *angle)
{
	*angle = get_branch_angle(a, dir);
	float inc = 2.f*M_PI/t;
	int min = *lb = *ub = (int)round(*angle/inc);
	int i;

	for (i = 1; i < (t)/2; i++) {
		float a = absf(*angle - (*lb - 1.f)*inc - inc/2.f);
		float b = absf(*angle - (*ub + 1.f)*inc - inc/2.f);
		if (a < b)
			(*lb)--;
		else
			(*ub)++;
	}

	*lb = *lb < 0 ? *lb+t : *lb;
	*ub = *ub < t ? *ub : *ub-t;
}

/** This limits the max number of vertices per cross section to 180. */
int is_dichotomous_twisted(float angle, int t)
{
	int m = angle*180.f/M_PI - 90;
	int l = 360.f/t;
	return m%l >= l/2;
}

/** Connects three cross sections labeled a, b, and c. */
void connect_dichotomous(int a, int b, int c, int t, node *n)
{
	int ub, lb;
	float angle;
	int twisted;
	int x, y;
	int i;
	get_bounds(n[1].lines[0].direction, t, a, &lb, &ub, &angle);
	twisted = is_dichotomous_twisted(angle, t);

	a /= 6;
	b /= 6;
	c /= 6;

	if (lb > ub)
		ebo_index += add_rect_seg(&ebo[ebo_index], a, b, ub+1, lb-1, t);
	else {
		ebo_index += add_rect_seg(&ebo[ebo_index], a, b, 0, lb-1, t);
		ebo_index += add_rect_seg(&ebo[ebo_index], a, b, ub+1, t, t);
	}

	if (lb < ub)
		ebo_index += add_rect_seg(&ebo[ebo_index], a, c, lb, ub, t);
	else {
		ebo_index += add_rect_seg(&ebo[ebo_index], a, c, 0, ub, t);
		ebo_index += add_rect_seg(&ebo[ebo_index], a, c, lb, t, t);
	}

	x = ub+1 >= t ? ub+1-t : ub+1;
	ebo[ebo_index++] = b + x;
	ebo[ebo_index++] = a + x;
	ebo[ebo_index++] = c + x;
	y = x;
	if (twisted && t%2 != 0)
		y = y+1 < t ? y+1 : 0;
	for (i = 0; i < t/2 + (twisted || t%2 == 0 ? 0 : 1); i++) {
		if (twisted) {
			ebo[ebo_index++] = b + (x-1 < 0 ? t-1 : x-1);
			ebo[ebo_index++] = b + x;
			ebo[ebo_index++] = c + y;
			y = y+1 < t ? y+1 : 0;
			x = x-1 < 0 ? t-1 : x-1;
		} else {
			ebo[ebo_index++] = c + x;
			ebo[ebo_index++] = c + (x+1 < t ? x+1 : 0);
			ebo[ebo_index++] = b + y;
			y = y-1 < 0 ? t-1 : y-1;
			x = x+1 < t ? x+1 : 0;
		}
	}

	ebo[ebo_index++] = a + lb;
	ebo[ebo_index++] = b + lb;
	ebo[ebo_index++] = c + lb;
	x = y = lb;
	if (!twisted && t%2 != 0)
		y = y-1 < 0 ? t-1 : y-1;
	for (i = 0; i < t/2 + (!twisted || t%2 == 0 ? 0 : 1); i++) {
		if (twisted) {
			ebo[ebo_index++] = c + (x-1 < 0 ? t-1 : x-1);
			ebo[ebo_index++] = c + x;
			ebo[ebo_index++] = b + y;
			y = y+1 < t ? y+1 : 0;
			x = x-1 < 0 ? t-1 : x-1;
		} else {
			ebo[ebo_index++] = b + x;
			ebo[ebo_index++] = b + (x+1 < t ? x+1 : 0);
			ebo[ebo_index++] = c + y;
			y = y-1 < 0 ? t-1 : y-1;
			x = x+1 < t ? x+1 : 0;
		}
	}
}

int add_branch(node *stem, node *parent, float offset);

void add_dichotomous(node *n, node *p, int a_index)
{
	struct line_t *l = &p->lines[p->line_count-1];
	float offset = l->length - n->radius;
	int b_index = vbo_index;

	add_cross_section_o(&vbo[a_index], p, l, offset);
	if (!add_branch(&n[0], p, 0.05)) {
		overflow = 1;
		return;
	}

	if (get_dichotomous_ecount(&n[0]) + ebo_index > ebo_size) {
		overflow = 1;
		return;
	}
	connect_dichotomous(a_index, b_index, vbo_index, p->resolution, n);

	if(!add_branch(&n[1], p, 0.05)) {
		overflow = 1;
		return;
	}
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
			if (overflow)
				return 0;
			dichotomous++;
			i++;
		} else
			if (!add_branch(&stem->branches[i], stem, 0.0f)) {
				overflow = 1;
				return 0;
			}
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

int add_branch(node *stem, node *parent, float offset)
{
	unsigned short l_index;
	unsigned short r_index;
	int prev_index;
	int i, j;
	float p;

	if (stem == NULL)
		return;

	for (i = 0; i < stem->cross_sections - 1; i++) {
		if (get_branch_vcount(stem) + vbo_index > vbo_size)
			return 0;
		else if (get_branch_ecount(stem) + ebo_index > ebo_size)
			return 0;

		p = offset_to_percent(stem, offset, i);
		add_cross_section(&vbo[vbo_index], stem, p);
		prev_index = vbo_index;
		vbo_index += get_branch_vcount(stem);
		add_triangle_ring(&ebo[ebo_index], prev_index/6,
				vbo_index/6, stem->resolution);
		ebo_index += get_branch_ecount(stem);
	}

	if (get_branch_vcount(stem) + vbo_index > vbo_size)
		return 0;

	prev_index = vbo_index;
	vbo_index += get_branch_vcount(stem);
	if (add_subbranches(stem, prev_index) == 0) {
		if (overflow)
			return 0;
		if (get_cap_ecount(stem) + ebo_index > ebo_size)
			return 0;

		add_cross_section(&vbo[prev_index], stem, 1.0f);
		cap_branch(prev_index/6, stem->resolution);
	}

	return 1;
}

int build_model(float *vb, int vb_size, unsigned short *eb, int eb_size,
		node *root)
{
	int i;
	vbo = vb;
	ebo = eb;
	vbo_size = vb_size;
	ebo_size = eb_size;
	vbo_index = 0;
	ebo_index = 0;
	overflow = 0;

	if(!add_branch(root, NULL, 0.0f))
		return 0;
	else
		return 1;
}

int get_vbo_size()
{
	return vbo_index/6;
}

int get_ebo_size()
{
	return ebo_index;
}

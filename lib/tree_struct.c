/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "tree_struct.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

static int max_branch_depth;
static tree_data *td;

struct position_t get_path_start_position(node *n)
{
	struct position_t pos = {-1, 0.0f};
	int i;

	for (i = 0; i < n->line_count; i++) {
		float end_y = get_line_end_point(&n->lines[i]).y;

		if (end_y >= td->crown_base_height) {
			pos.t = td->crown_base_height - n->lines[i].start.y;
			pos.index = i;
			break;
		}
	}

	return pos;
}

int increment_position(node *n, struct position_t *pos, float distance)
{
	distance += pos->t;
	while (distance >= n->lines[pos->index].length) {
		if (++pos->index >= n->line_count)
			return 0;
		distance -= n->lines[pos->index-1].length;
	}
	pos->t = distance;
	return 1;
}

vec3 get_branch_direction(node *n, node *p)
{
	float ax;
	float ay;
	vec3 v = {0.0f, 1.0f, 0.0f};
	mat4 r;

	float ratio = n->radius/p->radius;
	ax = (float)rand() / RAND_MAX * ratio * 4.f;
	ay = (float)rand() / RAND_MAX;
	r = tm_rotate_xy(M_PI*0.1f + ax, ay*M_PI*2.0f);
	tm_transform(&v, &r, 0.0f);
	return v;
}

void get_dichotomous_directions(node *n, vec3 *d)
{
	float ax = (float)rand() / RAND_MAX / 2.0f;
	float ay = (float)rand() / (RAND_MAX) + M_PI*.25f;
	vec3 a1 = n->lines[n->line_count-1].direction;
	vec3 a2 = {a1.x, 0, a1.z};
	vec3 b = tm_cross_vec3(&a1, &a2);
	b = tm_rotate_around_axis(&b, &a1, ay);
	d[0] = tm_rotate_around_axis(&a1, &b, ax);
	d[1] = tm_rotate_around_axis(&a1, &b, -ax);
}

void set_path(node *n, vec3 *start, vec3 *direction)
{
	int i;
	float len = (15*n->radius);

	n->line_count = 3;
	n->lines = malloc(sizeof(struct line_t) * n->line_count);
	n->lines[0].start = (tm_vec3){0.f, 0.f, 0.f};
	n->lines[0].length = len;
	n->lines[0].direction = *direction;
	n->glob_pos = *start;

	for (i = 1; i < n->line_count; i++) {
		struct line_t *l = &n->lines[i-1];
		n->lines[i].start = get_line_point(l, l->length);
		n->lines[i].length = len;
		n->lines[i].direction = l->direction;
		n->lines[i].direction.x += 0.02;
		n->lines[i].direction.z -= 0.05;
	}
}

void set_radius_curve(node *n)
{
	if (n->radius_curve_size > 0)
		free(n->radius_curve);
	n->radius_curve = malloc(sizeof(vec3)*8);
	n->radius_curve_size = 8;
	n->radius_curve[0] = (tm_vec3){0.0f, -0.3f, 1.0f};
	n->radius_curve[1] = (tm_vec3){0.0f, -0.3f, 0.75f};
	n->radius_curve[2] = (tm_vec3){0.05f, -0.3f, 0.75f};
	n->radius_curve[3] = (tm_vec3){0.175f, -0.3f, 0.75f};
	n->radius_curve[4] = (tm_vec3){0.175f, -0.3f, 0.75f};
	n->radius_curve[5] = (tm_vec3){0.3f, -0.3f, 0.75f};
	n->radius_curve[6] = (tm_vec3){0.75f, -0.3f, 0.25f};
	n->radius_curve[7] = (tm_vec3){1.0f, -0.3f, 0.0f};
}

void set_branch_curve(node *n)
{
	if (n->branch_curve_size > 0)
		n->branch_curve = realloc(n->branch_curve, sizeof(tm_vec3)*4);
	else
		n->branch_curve = malloc(sizeof(tm_vec3)*4);
	n->branch_curve_size = 4;
	n->branch_curve[0] = (tm_vec3){0.0f, -0.3f, 1.0f};
	n->branch_curve[1] = (tm_vec3){0.25f, -0.3f, 1.0f};
	n->branch_curve[2] = (tm_vec3){0.75f, -0.3f, 1.0f};
	n->branch_curve[3] = (tm_vec3){1.0f, -0.3f, 1.0f};
}

void add_lateral_branches(node *stem, struct position_t pos)
{
	node *n;
	int i;

	if (pos.index < 0 || stem->branch_density <= 0)
		return;

	for (i = stem->branch_count; i < stem->branch_capacity; i++) {
		n = &stem->branches[i];
		n->radius = stem->radius * 0.35f / pow(i + 1.0f, 0.3f);
		n->cross_sections = 4;
		n->resolution = stem->resolution-4 < 5 ? 5 : stem->resolution-4;
		n->dichotomous_start = -1;
		n->terminal = 0;
		n->branch_density = 0.0f;
		vec3 start = get_line_point(&stem->lines[pos.index], pos.t);
		start = tm_add_vec3(&start, &stem->glob_pos);
		vec3 direction = get_branch_direction(n, stem);
		set_path(n, &start, &direction);
		set_radius_curve(n);
		set_branch_curve(n);
		n->branch_capacity = 10;
		n->branch_count = 0;
		n->branches = new_nodes(n->branch_capacity);
		n->depth = stem->depth + 1;

		if ((float)rand() / RAND_MAX > 0.8) {
			n->min_radius = 0.02f;
			add_dichotomous_branches(n);
		} else
			n->min_radius = 0.01f;

		if (++stem->branch_count == stem->branch_capacity)
			expand_branches(stem);
		if (!increment_position(stem, &pos, 1.0f/stem->branch_density))
			break;
	}
}

void set_dichotomous_branch(node *n, node *p, vec3 direction)
{
	vec3 start = get_line_end_point(&p->lines[p->line_count-1]);
	start = tm_add_vec3(&p->glob_pos, &start);
	n->radius = p->min_radius;
	n->min_radius = 0.01f;
	n->cross_sections = 4;
	n->resolution = p->resolution;
	n->branch_capacity = 0;
	n->dichotomous_start = -1;
	n->terminal = 1;
	n->branch_density = 0.0f;
	n->depth = p->depth + 1;
	set_path(n, &start, &direction);
	set_radius_curve(n);
	set_branch_curve(n);
}

vec3 add_dichotomous_branches(node *stem)
{
	node *n;
	vec3 d[2];

	if (stem->branch_count + 2 >= stem->branch_capacity)
		expand_branches(stem);

	get_dichotomous_directions(stem, d);
	stem->dichotomous_start = stem->branch_count;
	n = &stem->branches[stem->branch_count++];
	set_dichotomous_branch(n, stem, d[0]);
	n = &stem->branches[stem->branch_count++];
	set_dichotomous_branch(n, stem, d[1]);
}

node *new_tree_structure(tree_data *data, node *root)
{
	td = data;
	root->branch_capacity = 20;
	root->branches = new_nodes(root->branch_capacity);
	root->depth = 1;
	root->min_radius = 0.04f;
	root->terminal = 0;
	root->branch_density = 2.0f;
	vec3 origin = {0.0f, 0.0f, 0.0f};
	vec3 direction = {0.0f, 1.0f, 0.0f};
	set_path(root, &origin, &direction);
	set_radius_curve(root);
	set_branch_curve(root);

	add_lateral_branches(root, get_path_start_position(root));
	add_dichotomous_branches(root);
}

void free_tree_structure(node *root)
{
	remove_nodes(root);
}

void reset_tree_structure(node *root)
{
	int i = 0;
	for (; i < root->branch_count; i++)
		free_tree_structure(&root->branches[i]);
	if (root->line_count > 0)
		free(root->lines);
}

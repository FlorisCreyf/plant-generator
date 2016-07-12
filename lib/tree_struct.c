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

vec3 get_start_position(node *n, int i)
{
	float offset;
	float percent = (i+1.0f)/(n->branch_count+1.0f);
	int line = get_line(n, percent, &offset);
	return get_line_point(&n->lines[line], offset);
}

vec3 get_branch_direction()
{
	float ax;
	float ay;
	vec3 v = {0.0f, 1.0f, 0.0f};
	mat4 r;

	ax = (float)rand() / RAND_MAX;
	ay = (float)rand() / RAND_MAX;
	r = bt_rotate_xy(M_PI*0.5f - ax, ay*M_PI*2.0f);
	bt_transform(&v, &r, 0.0f);
	return v;
}

void get_dichotomous_directions(node *n, vec3 *d)
{
	float ax = (float)rand() / RAND_MAX + 0.1;
	float ay = (float)rand() / (RAND_MAX) + M_PI*.25f;
	vec3 a1 = n->lines[n->line_count-1].direction;
	vec3 a2 = {a1.x, 0, a1.z};
	vec3 b = bt_cross_vec3(&a1, &a2);
	b = bt_rotate_around_axis(&b, &a1, ay);
	d[0] = bt_rotate_around_axis(&a1, &b, ax);
	d[1] = bt_rotate_around_axis(&a1, &b, -ax);
}

void remove_nodes(node *stem)
{
	int i;
	if (stem != NULL) {
		for (i = 0; i < stem->branch_count; i++)
			remove_nodes(&stem->branches[i]);

		if (stem->branch_capacity > 0)
			free(stem->branches);
		if (stem->line_count > 0)
			free(stem->lines);
	}
}

void set_path(node *n, vec3 *start, vec3 *direction)
{
	int i;
	float len = (15*n->radius);

	n->line_count = 3;
	n->lines = malloc(sizeof(struct line_t) * n->line_count);
	n->lines[0].start = *start;
	n->lines[0].length = len;
	n->lines[0].direction = *direction;

	for (i = 1; i < n->line_count; i++) {
		struct line_t *l = &n->lines[i-1];
		n->lines[i].start = get_line_point(l, l->length);
		n->lines[i].length = len;
		n->lines[i].direction = l->direction;
		n->lines[i].direction.x += 0.02;
		n->lines[i].direction.z -= 0.05;
	}
}

vec3 add_dichotomous_branches(node *stem);

void add_lateral_branches(node *stem)
{
	node *n;
	float diff;
	const int total = 17;
	int count = stem->branch_count;
	int i;

	stem->branch_count += total;

	for (i = count; i < total + count; i++) {
		diff = pow(i + 1.0f, 0.3f);
		n = &stem->branches[i];
		n->radius = stem->radius * 0.35f / diff;
		n->cross_sections = 3;
		n->resolution = stem->resolution-4 < 5 ? 5 : stem->resolution-4;
		n->dichotomous_start = -1;
		n->terminal = 0;
		vec3 start = get_start_position(stem, i);
		vec3 direction = get_branch_direction();
		set_path(n, &start, &direction);
		n->branch_count = 0;
		n->branch_capacity = 10;
		n->branches = malloc(sizeof(node) * n->branch_capacity);
		if ((float)rand() / RAND_MAX > 0.8) {
			n->min_radius = 0.02f;
			add_dichotomous_branches(n);
		} else
			n->min_radius = 0.01f;
	}
}

void set_dichotomous_branch(node *n, node *p, vec3 direction)
{
	vec3 start = get_line_end_point(&p->lines[p->line_count-1]);

	n->radius = p->min_radius;
	n->min_radius = 0.01f;
	n->cross_sections = 3;
	n->resolution = p->resolution;
	n->branch_count = 0;
	n->branch_capacity = 0;
	n->dichotomous_start = -1;
	n->terminal = 1;
	set_path(n, &start, &direction);
}

vec3 add_dichotomous_branches(node *stem)
{
	node *n;
	vec3 d[2];
	get_dichotomous_directions(stem, d);
	stem->dichotomous_start = stem->branch_count;
	n = &stem->branches[stem->branch_count++];
	set_dichotomous_branch(n, stem, d[0]);
	n = &stem->branches[stem->branch_count++];
	set_dichotomous_branch(n, stem, d[1]);
}

node *new_tree_structure(node *root)
{
	root->branch_count = 0;
	root->branch_capacity = 20;
	root->branches = malloc(sizeof(node) * root->branch_capacity);
	root->depth = 1;
	root->min_radius = 0.04f;
	root->cross_sections = 6;
	root->terminal = 0;
	vec3 origin = {0.0f, 0.0f, 0.0f};
	vec3 direction = {0.0f, 1.0f, 0.0f};
	set_path(root, &origin, &direction);

	add_lateral_branches(root);
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

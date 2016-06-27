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

void add_lateral_branches(node *stem)
{
	node *n;
	float diff;
	const int total = 7;
	int count = stem->branch_count;
	int i;

	stem->branch_count += total;

	for (i = count; i < total + count; i++) {
		diff = pow(i + 1.0f, 0.3f);
		n = &stem->branches[i];
		n->min_radius = 0.01f;
		n->radius = stem->radius * 0.35f / diff;
		n->cross_sections = 3;
		n->resolution = stem->resolution - 4;
		n->branch_count = 0;
		n->branch_capacity = 0;

		vec3 start = get_start_position(stem, i);
		vec3 direction = get_branch_direction();
		set_path(n, &start, &direction);
	}
}

void set_dichotomous_branch(node *n, node *p, vec3 direction)
{
	vec3 start = get_line_end_point(&p->lines[p->line_count-1]);

	n->radius = p->radius * 0.2f;
	n->min_radius = 0.01f;
	p->min_radius = p->min_radius > n->radius ? p->min_radius : n->radius;
	n->cross_sections = 3;
	n->resolution = p->resolution;
	n->branch_count = 0;
	n->branch_capacity = 0;
	set_path(n, &start, &direction);
}

vec3 add_dichotomous_branches(node *stem)
{
	node *n;
	vec3 direction = get_branch_direction();

	n = &stem->branches[stem->branch_count++];
	set_dichotomous_branch(n, stem, direction);

	direction.x = -direction.x;
	direction.z = -direction.z;
	n = &stem->branches[stem->branch_count++];
	set_dichotomous_branch(n, stem, direction);
}

node *new_tree_structure(tree_data *td)
{
	node *root = (node *)malloc(sizeof(struct node_t));
	root->branch_count = 0;
	root->branch_capacity = 10;
	root->branches = malloc(sizeof(node) * root->branch_capacity);

	root->depth = 1;
	root->min_radius = 0.0f;
	root->radius = td->trunk_radius;
	root->resolution = td->resolution;
	root->cross_sections = 6;

	vec3 origin = {0.0f, 0.0f, 0.0f};
	vec3 direction = {0.0f, 1.0f, 0.0f};
	set_path(root, &origin, &direction);

	max_branch_depth = td->max_branch_depth;

	add_lateral_branches(root);
	add_dichotomous_branches(root);

	return root;
}

void free_tree_structure(node *root)
{
	remove_nodes(root);
}

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

vec3 get_position(node *stem, node *parent, float i)
{
	mat4 rot = bt_quat_to_mat4(&(parent->direction));
	float height = (i+1)/(parent->branch_count+1)*parent->branch_length;
	vec3 pos = (vec3){0.0f, height, 0.0f};
	bt_transform(&pos, &rot, 1.0f);
	return pos;
}

quat get_branch_direction()
{
	float ax, ay;
	quat qx, qy;

	ax = (float)rand() / RAND_MAX;
	ay = (float)rand() / RAND_MAX;

	qx = bt_from_axis_angle(1.0f, 0.0f, 0.0f, M_PI / 2.0f - ax);
	qy = bt_from_axis_angle(0.0f, 1.0f, 0.0f, ay * M_PI * 2.0f);

	return bt_mult_quat(&qy, &qx);
}

void remove_nodes(node *stem)
{
	int i;
	if (stem != NULL) {
		for (i = 0; i < stem->branch_count; i++)
			remove_nodes(&stem->branches[i]);

		if (stem->branch_count > 0)
			free(stem->branches);
	}
}

void add_lateral_branch(node *stem)
{

}

void add_lateral_branches(node *stem)
{
	node *n;
	float diff;
	const int total = 7;
	int i;

	stem->branches = malloc(sizeof(node) * total);
	stem->branch_count += total;

	for (i = 0; i < total; i++) {
		diff = pow(i + 1.0f, 0.3f);
		n = &stem->branches[i];
		n->radius = stem->radius * 0.35f / diff;
		n->cross_sections = 3;
		n->branch_resolution = stem->branch_resolution - 4;
		n->branch_length = stem->branch_length * 0.5f / diff;
		n->direction = get_branch_direction();;
		n->position = get_position(n, stem, i);
		n->branch_count = 0;
	}
}

void add_dichotomous_branches(node *stem)
{
	quat direction = get_branch_direction();

	direction.x = -direction.x;
	direction.z = -direction.z;

}

node *new_tree_structure(tree_data *td)
{
	node *root = (node *)malloc(sizeof(struct node_tag));
	root->branch_depth = 1;
	root->direction = (quat){0.0f, 0.0f, 0.0f, 0.0f};
	root->position = (vec3){0.0f, 0.0f, 0.0f};
	root->radius = td->trunk_radius;
	root->branch_resolution = td->resolution;
	root->cross_sections = 6;
	root->branch_length = 5.0f;
	root->branch_count = 0;

	max_branch_depth = td->max_branch_depth;

	add_lateral_branches(root);

	return root;
}

void free_tree_structure(node *root)
{
	remove_nodes(root);
}

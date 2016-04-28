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

static int max_branch_depth;
static int vertex_count;
static int index_count;

bt_vec3 get_position(node *stem, node *parent)
{
	bt_quat q_rot = bt_mult_quat(&(parent->direction), &(stem->direction));
	bt_mat4 m_rot = bt_quat_to_mat4(&q_rot);
	bt_vec3 pos = (bt_vec3){0.0f, 1.0f, 0.0f};
	bt_transform(&pos, &m_rot, 1.0f);
	return pos;
}

bt_quat get_branch_direction()
{
	float ax, ay;
	bt_quat qx, qy;
	
	ax = rand() / RAND_MAX * 20;
	ay = rand() / RAND_MAX * 20;
	qx = bt_from_axis_angle(1.0f, 0.0f, 0.0f, ax);
	qy = bt_from_axis_angle(0.0f, 1.0f, 0.0f, ay);
	
	return bt_mult_quat(&qy, &qx);
}

node *add_node(node *parent, char direction)
{
	node *stem = (node *)malloc(sizeof(struct node_tag));
	stem->branch_depth = parent->branch_depth + 1;
	stem->radius = parent->radius * 0.8;
	stem->cross_sections = 0;
	stem->branch_resolution = parent->branch_resolution;
	stem->direction = get_branch_direction();
	stem->position = get_position(stem, parent);

	if (max_branch_depth <= stem->branch_depth) {
		stem->left = NULL;
		stem->right = NULL;
	} else {
		stem->left = add_node(stem, 'l');
		stem->right = add_node(stem, 'r');
	}

	vertex_count += stem->branch_resolution;

	return stem;
}

void remove_nodes(node *stem)
{
	if (stem != NULL) {
		remove_nodes(stem->left);
		remove_nodes(stem->right);
		free(stem);
	}
}

node *new_tree_structure(tree_data *td)
{	
	node *root = (node *)malloc(sizeof(struct node_tag));
	root->branch_depth = 1;
	root->direction = get_branch_direction();
	root->position = (bt_vec3){0.0f, 0.0f, 0.0f};
	root->radius = td->trunk_radius;
	root->branch_resolution = td->resolution;
	root->cross_sections = 6;
	root->branch_length = 6.0f;

	vertex_count = root->branch_resolution;
	index_count = vertex_count * 3;
	max_branch_depth = td->max_branch_depth;

	root->left = add_node(root, 'l');
	root->right = add_node(root, 'r');

	return root;
}

void free_tree_structure(node *root)
{
	remove_nodes(root);
}

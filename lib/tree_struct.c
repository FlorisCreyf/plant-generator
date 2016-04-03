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

typedef bt_vec3 vec3;

static int max_branch_depth;
static int vertex_count;
static int index_count;

node *add_node(node *parent, char direction)
{
	node *stem = (node *)malloc(sizeof(struct node_tag));
	stem->branch_depth = parent->branch_depth + 1;
	stem->radius = parent->radius * 0.8;
	stem->branch_resolution = parent->branch_resolution;
	stem->direction = (bt_vec3){0.0f, 1.0f, 0.0f};

	if (max_branch_depth >= stem->branch_depth) {
		stem->left = NULL;
		stem->right = NULL;
	} else {
		stem->left = add_node(stem, 'l');
		stem->right = add_node(stem, 'r');
	}

	vertex_count += stem->branch_resolution;

	return stem;
}

void remove_node(node *stem)
{
	if (stem != NULL) {
		remove_node(stem->left);
		remove_node(stem->right);
		free(stem);
	}
}

node *new_tree_structure(tree_data *td)
{	
	node *root = (node *)malloc(sizeof(struct node_tag));
	root->branch_depth = 1;
	root->direction = (vec3){0.0f, 1.0f, 0.0f};
	root->radius = td->trunk_radius;
	root->branch_resolution = td->resolution;

	vertex_count = root->branch_resolution;
	index_count = vertex_count * 3;
	max_branch_depth = td->max_branch_depth;

	root->left = add_node(root, 'l');
	root->right = add_node(root, 'r');

	return root;
}

void free_tree_structure(node *root)
{
	remove_node(root);
}

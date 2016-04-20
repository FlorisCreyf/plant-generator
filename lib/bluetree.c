/* 
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "bluetree.h"
#include "tree_struct.h"
#include "model_builder.h"
#include "tree_data.h"
#include <stdlib.h>

struct bt_tree_tag {
	node *root;
	tree_data td;
	float *vertex_buffer;
	unsigned short *element_buffer;
};

bt_tree bt_new_tree()
{
	bt_tree tree = (bt_tree)malloc(sizeof(struct bt_tree_tag));
	tree->root = NULL;
	tree->td.vbo_size = 0;
	tree->td.ebo_size = 0;
	return tree;
}

void bt_delete_tree(bt_tree tree)
{
	free_tree_structure(tree->root);	
	free(tree);
}

void bt_set_trunk_radius(bt_tree tree, float radius)
{
	tree->td.trunk_radius = radius;
}

void bt_set_resolution(bt_tree tree, int resolution)
{
	tree->td.resolution = resolution;
}

void bt_set_max_branch_depth(bt_tree tree, int depth)
{
	tree->td.max_branch_depth = depth;
}

void bt_generate_structure(bt_tree tree)
{
	free_tree_structure(tree->root);
	tree->root = new_tree_structure(&(tree->td));
}

int bt_get_vbo_size(bt_tree tree)
{
	return get_vbo_size();
}

int bt_get_ebo_size(bt_tree tree)
{
	return tree->td.ebo_size;
}

void bt_generate_mesh(bt_tree tree, float *vb, int vb_size,
		unsigned short *eb, int eb_size)
{
	build_model(vb, vb_size, eb, eb_size, tree->root);
}


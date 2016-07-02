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
#include <stdio.h>

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
	if (tree->root)
		tree->root->radius = radius;
}

void change_dichotomous_resolution(node *stem, int r)
{
	int i;
	vec3 n;
	vec3 p = get_line_end_point(&(stem->lines[stem->line_count-1]));

	for (; i < stem->branch_count; i++) {
		n = stem->branches[i].lines[0].start;
		if (p.x == n.x && p.y == n.y && p.z == n.z) {
			stem->branches[i].resolution = r;
			change_dichotomous_resolution(&stem->branches[i], r);
		}
	}
}

void bt_set_resolution(bt_tree tree, int resolution)
{
	tree->td.resolution = resolution;
	if (tree->root) {
		tree->root->resolution = resolution;
		change_dichotomous_resolution(tree->root, resolution);
	}
}

void bt_set_cross_sections(bt_tree tree, int sections)
{
	tree->td.cross_sections = sections;
	if (tree->root) {
		tree->root->cross_sections = sections;
	}
}

void bt_set_max_branch_depth(bt_tree tree, int depth)
{
	tree->td.max_branch_depth = depth;
}

int bt_get_cross_sections(bt_tree tree)
{
	return tree->root ? tree->root->cross_sections : -1;
}

int bt_get_resolution(bt_tree tree)
{
	return tree->root ? tree->root->resolution : -1;
}

float bt_get_radius(bt_tree tree)
{
	return tree->root ? tree->root->radius : -1.0f;
}

void bt_generate_structure(bt_tree tree)
{
	free_tree_structure(tree->root);
	tree->root = new_tree_structure(&(tree->td));
}

int bt_get_vbo_size(bt_tree tree)
{
	return tree->td.vbo_size;
}

int bt_get_ebo_size(bt_tree tree)
{
	return tree->td.ebo_size;
}

void bt_generate_mesh(bt_tree tree, float *vb, int vb_size,
		unsigned short *eb, int eb_size)
{
	build_model(vb, vb_size, eb, eb_size, tree->root);
	tree->td.ebo_size = get_ebo_size();
	tree->td.vbo_size = get_vbo_size();
}

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
	tree->root = (node *)malloc(sizeof(struct node_t));
	tree->root->branch_count = 0;
	tree->root->line_count = 0;
	tree->td.vbo_size = 0;
	tree->td.ebo_size = 0;
	return tree;
}

void bt_delete_tree(bt_tree tree)
{
	free_tree_structure(tree->root);
	free(tree);
}

node *find_node(node *n, int *i, int id)
{
	node *m;
	int j = 0;

	if ((*i)++ == id)
		return n;

	for (; j < n->branch_count; j++) {
		m = find_node(&(n->branches[j]), i, id);
		if (m != NULL)
			return m;
	}

	return NULL;
}

node *get_node(node *n, int id)
{
	int i = 0;
	return find_node(n, &i, id);
}

void bt_set_trunk_radius(bt_tree tree, int id, float radius)
{
	node *n = get_node(tree->root, id);
	if (n)
		n->radius = radius;
}

void change_dichotomous_resolution(node *stem, int r)
{
	int i;
	if (stem->branch_count == 0 && stem->dichotomous_start < 0)
		return;
	for (i = stem->dichotomous_start; i < stem->branch_count; i++) {
		stem->branches[i].resolution = r;
		change_dichotomous_resolution(&stem->branches[i], r);
	}
}

void bt_set_resolution(bt_tree tree, int id, int resolution)
{
	node *n = get_node(tree->root, id);
	if (n) {
		n->resolution = resolution;
		change_dichotomous_resolution(n, resolution);
	}
}

void bt_set_cross_sections(bt_tree tree, int id, int sections)
{
	node *n = get_node(tree->root, id);
	if (n)
		n->cross_sections = sections;
}

void bt_set_max_branch_depth(bt_tree tree, int depth)
{
	tree->td.max_branch_depth = depth;
}

int bt_get_cross_sections(bt_tree tree, int id)
{
	node *n = get_node(tree->root, id);
	return n ? n->cross_sections : -1;
}

int bt_get_resolution(bt_tree tree, int id)
{
	node *n = get_node(tree->root, id);
	return n ? n->resolution : -1;
}

float bt_get_radius(bt_tree tree, int id)
{
	node *n = get_node(tree->root, id);
	return n ? n->radius : -1.0f;
}

bt_aabb bt_get_bounding_box(bt_tree tree, int id)
{
	node *n = get_node(tree->root, id);
	return n ? n->bounds : (bt_aabb){0, 0, 0, 0, 0, 0};
}

int bt_get_ebo_start(bt_tree tree, int id)
{
	node *n = get_node(tree->root, id);
	return n->ebo_start;
}

int bt_get_ebo_end(bt_tree tree, int id)
{
	node *n = get_node(tree->root, id);
	return n->ebo_end;
}

int bt_is_terminal_branch(bt_tree tree, int id)
{
	node *n = get_node(tree->root, id);
	return n->terminal;
}

void bt_generate_structure(bt_tree tree)
{
	reset_tree_structure(tree->root);
	new_tree_structure(tree->root);
}

int bt_get_vbo_size(bt_tree tree)
{
	return tree->td.vbo_size;
}

int bt_get_ebo_size(bt_tree tree)
{
	return tree->td.ebo_size;
}

int bt_generate_mesh(bt_tree tree, float *vb, int vb_size,
		unsigned short *eb, int eb_size)
{
	int status = build_model(vb, vb_size, eb, eb_size, tree->root);
	tree->td.ebo_size = get_ebo_size();
	tree->td.vbo_size = get_vbo_size();
	return status;
}

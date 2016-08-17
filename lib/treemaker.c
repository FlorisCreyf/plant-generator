/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "treemaker.h"
#include "tree_struct.h"
#include "model_builder.h"
#include "tree_data.h"
#include <stdlib.h>
#include <stdio.h>

struct tm_tree_tag {
	node *root;
	tree_data td;
	float *vertex_buffer;
	unsigned short *element_buffer;
};

tm_tree tm_new_tree()
{
	tm_tree tree = (tm_tree)malloc(sizeof(struct tm_tree_tag));
	tree->root = (node *)malloc(sizeof(struct node_t));
	tree->root->branch_count = 0;
	tree->root->line_count = 0;
	tree->root->radius_curve_size = 0;
	tree->td.vbo_size = 0;
	tree->td.ebo_size = 0;
	return tree;
}

void tm_delete_tree(tm_tree tree)
{
	if (tree != NULL) {
		free_tree_structure(tree->root);
		free(tree);
	}
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

void tm_set_radius(tm_tree tree, int id, float radius)
{
	node *n = get_node(tree->root, id);
	if (n)
		n->radius = radius;
}

void tm_set_radius_curve(tm_tree tree, int id, tm_vec3 *controls, int size)
{
	node *n = get_node(tree->root, id);
	if (n->radius_curve_size != 0)
		free(n->radius_curve);
	n->radius_curve = malloc(sizeof(tm_vec3)*size);
	memcpy(n->radius_curve, controls, sizeof(tm_vec3)*size);
	n->radius_curve_size = size;
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

void tm_set_resolution(tm_tree tree, int id, int resolution)
{
	node *n = get_node(tree->root, id);
	if (n) {
		n->resolution = resolution;
		change_dichotomous_resolution(n, resolution);
	}
}

void tm_set_cross_sections(tm_tree tree, int id, int sections)
{
	node *n = get_node(tree->root, id);
	if (n)
		n->cross_sections = sections;
}

void tm_set_max_branch_depth(tm_tree tree, int depth)
{
	tree->td.max_branch_depth = depth;
}

int tm_get_cross_sections(tm_tree tree, int id)
{
	node *n = get_node(tree->root, id);
	return n ? n->cross_sections : -1;
}

int tm_get_resolution(tm_tree tree, int id)
{
	node *n = get_node(tree->root, id);
	return n ? n->resolution : -1;
}

float tm_get_radius(tm_tree tree, int id)
{
	node *n = get_node(tree->root, id);
	return n ? n->radius : -1.0f;
}

void tm_get_radius_curve(tm_tree tree, int id, tm_vec3 **curve, int *size)
{
	node *n = get_node(tree->root, id);
	if (n) {
		*curve = n->radius_curve;
		*size = n->radius_curve_size;
	}
}

tm_aabb tm_get_bounding_box(tm_tree tree, int id)
{
	node *n = get_node(tree->root, id);
	return n ? n->bounds : (tm_aabb){0, 0, 0, 0, 0, 0};
}

int tm_get_ebo_start(tm_tree tree, int id)
{
	node *n = get_node(tree->root, id);
	return n->ebo_start;
}

int tm_get_ebo_end(tm_tree tree, int id)
{
	node *n = get_node(tree->root, id);
	return n->ebo_end;
}

int tm_is_terminal_branch(tm_tree tree, int id)
{
	node *n = get_node(tree->root, id);
	return n->terminal;
}

void tm_generate_structure(tm_tree tree)
{
	reset_tree_structure(tree->root);
	new_tree_structure(tree->root);
}

int tm_get_vbo_size(tm_tree tree)
{
	return tree->td.vbo_size;
}

int tm_get_ebo_size(tm_tree tree)
{
	return tree->td.ebo_size;
}

int tm_generate_mesh(tm_tree tree, float *vb, int vb_size,
		unsigned short *eb, int eb_size)
{
	int status = build_model(vb, vb_size, eb, eb_size, tree->root);
	tree->td.ebo_size = get_ebo_size();
	tree->td.vbo_size = get_vbo_size();
	return status;
}

/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef BLUETREE_H
#define BLUETREE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "collision.h"

typedef struct bt_tree_tag *bt_tree;

bt_tree bt_new_tree();
void bt_delete_tree(bt_tree tree);

void bt_set_radius(bt_tree tree, int id, float radius);
void bt_set_radius_curve(bt_tree tree, int id, bt_vec3 *controls, int size);
void bt_set_cross_sections(bt_tree tree, int id, int sections);
void bt_set_resolution(bt_tree tree, int id, int resolution);
void bt_set_max_branch_depth(bt_tree tree, int depth);

int bt_get_cross_sections(bt_tree tree, int id);
int bt_get_resolution(bt_tree tree, int id);
float bt_get_radius(bt_tree tree, int id);
bt_aabb bt_get_bounding_box(bt_tree tree, int id);
int bt_get_ebo_start(bt_tree tree, int id);
int bt_get_ebo_end(bt_tree tree, int id);
int bt_is_terminal_branch(bt_tree tree, int id);

void bt_generate_structure(bt_tree tree);

int bt_get_vbo_size(bt_tree tree);
int bt_get_ebo_size(bt_tree tree);

int bt_generate_mesh(bt_tree tree, float *vertex_buffer, int vb_size,
		unsigned short *element_buffer, int eb_size);

#ifdef __cplusplus
}
#endif

#endif /* BLUETREE_H  */

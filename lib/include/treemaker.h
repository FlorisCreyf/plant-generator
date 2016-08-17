/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef TREEMAKER_H
#define TREEMAKER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "collision.h"

typedef struct tm_tree_tag *tm_tree;

tm_tree tm_new_tree();
void tm_delete_tree(tm_tree tree);

void tm_set_radius(tm_tree tree, int id, float radius);
float tm_get_radius(tm_tree tree, int id);

void tm_set_radius_curve(tm_tree tree, int id, tm_vec3 *controls, int size);
void tm_get_radius_curve(tm_tree tree, int id, tm_vec3 **curve, int *size);

void tm_set_cross_sections(tm_tree tree, int id, int sections);
int tm_get_cross_sections(tm_tree tree, int id);

void tm_set_resolution(tm_tree tree, int id, int resolution);
void tm_set_max_branch_depth(tm_tree tree, int depth);

int tm_get_resolution(tm_tree tree, int id);
tm_aabb tm_get_bounding_box(tm_tree tree, int id);
int tm_is_terminal_branch(tm_tree tree, int id);
int tm_get_ebo_start(tm_tree tree, int id);
int tm_get_ebo_end(tm_tree tree, int id);
int tm_get_vbo_size(tm_tree tree);
int tm_get_ebo_size(tm_tree tree);

void tm_generate_structure(tm_tree tree);
int tm_generate_mesh(tm_tree tree, float *vertex_buffer, int vb_size,
		unsigned short *element_buffer, int eb_size);

#ifdef __cplusplus
}
#endif

#endif /* TREEMAKER_H  */

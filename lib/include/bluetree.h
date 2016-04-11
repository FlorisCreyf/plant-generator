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

typedef struct bt_tree_tag *bt_tree;

bt_tree bt_new_tree();
void bt_delete_tree(bt_tree tree);

void bt_set_trunk_radius(bt_tree tree, float radius);
void bt_set_resolution(bt_tree tree, int resolution);
void bt_set_max_branch_depth(bt_tree tree, int depth);

void bt_generate_structure(bt_tree tree);

int bt_get_vbo_size(bt_tree tree);
int bt_get_ebo_size(bt_tree tree);

void bt_generate_mesh(bt_tree tree, float *vertex_buffer,
		unsigned short *element_buffer);

#ifdef __cplusplus
}
#endif

#endif /* BLUETREE_H  */


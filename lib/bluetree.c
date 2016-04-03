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
};

bt_tree *bt_new_tree()
{
	bt_tree *tree = (bt_tree *)malloc(sizeof(struct bt_tree_tag));
	return tree;
}

void bt_delete_tree(bt_tree *tree)
{
	free_tree_structure(tree->root);	
	free(tree);
}

void bt_generate_tree(bt_tree *tree, float *vertex_buffer,
		unsigned short *element_buffer)
{
	/* If bt_gen_tree() is called multiple times with the same struct: */
	free_tree_structure(tree->root);

	tree->root = new_tree_structure(&(tree->td));
	build_model(vertex_buffer, element_buffer, tree->root);
}


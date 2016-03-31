/* 
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include <stdlib.h>
#include "bluetree.h"
#include "tree_struct.h"

struct bt_tree_tag {
	node *root;
	unsigned int max_branch_depth;
	float tree_height;
};

bt_tree bt_new_tree()
{
	bt_tree tree = (bt_tree)malloc(sizeof(struct bt_tree_tag));
	return tree;
}

void bt_delete_tree(bt_tree tree)
{
	free(tree);
}


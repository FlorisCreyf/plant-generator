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

struct bt_tree_tag {
	bt_node *root;
	unsigned int max_branch_depth;
	float tree_height;
};

bt_tree *bt_new_tree()
{
	
}

void bt_delete_tree()
{

}

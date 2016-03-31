/* 
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "tree_struct.h"
#include <stdlib.h>

int max_branch_depth = 0;

node *addNode(node *parent)
{
	node *stem = (node*)malloc(sizeof(struct node_tag));
	stem->branch_depth = parent->branch_depth + 1;
	stem->radius = parent->radius * 0.8;
	stem->branch_resolution = parent->branch_resolution >= 3 ? parent->branch_resolution -1 : 3;
	stem->direction = (bt_vec3){0.0f, 1.0f, 0.0f};

	if (max_branch_depth >= stem->branch_depth) {
		stem->left = NULL;
		stem->right = NULL;
	} else {
		stem->left = addNode(stem);
		stem->right = addNode(stem);
	}
}

void removeNode(node *stem)
{
	if (stem != NULL) {
		removeNode(stem->left);
		removeNode(stem->right);
		free(stem);
	}
}


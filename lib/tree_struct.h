/* 
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef TREE_STRUCT
#define TREE_STRUCT

#include "vector.h"

typedef struct node_tag {
	float radius;
	float tapper;
	int branch_resolution;
	bt_vec3 direction;
	int branch_depth;
	struct node_tag *left;
	struct node_tag *right;
} node;

#endif

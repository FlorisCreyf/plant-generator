/* 
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef NODE_H
#define NODE_H

#include "vector.h"

typedef struct node_tag {
	float radius;
	float tapper;
	float branch_length;
	quat direction;
	vec3 position;
	int branch_depth;
	int branch_resolution;
	int cross_sections;
	int branch_count;
	struct node_tag *branches;
} node;

#endif /* NODE_H */


/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef TREE_DATA_H
#define TREE_DATA_H

typedef struct tree_data_tag {
	int vbo_size;
	int ebo_size;
	int resolution;
	int cross_sections;
	int max_branch_depth;
	float trunk_radius;
} tree_data;

#endif /* TREE_DATA_H */

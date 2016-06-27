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

struct point_t {
	float x;
	float y;
};

typedef struct line_t {
	vec3 start;
	vec3 direction;
	float length;
} line;

typedef struct node_t {
	int line_count;
	struct line_t *lines;

	int branch_count;
	int branch_capacity;
	struct node_t *branches;

	float radius;
	float min_radius;
	int depth;
	int resolution;
	int cross_sections;
} node;

vec3 get_line_point(struct line_t *l, float t);
vec3 get_line_end_point(struct line_t *l);
int get_line(node *n, float percent, float *offset);
float get_line_length(node *stem);

#endif /* NODE_H */

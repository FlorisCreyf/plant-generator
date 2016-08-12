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
#include "collision.h"

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
	int dichotomous_start;
	int terminal;

	float radius;
	float min_radius;
	bt_vec3 *radius_curve;
	int radius_curve_size;

	int depth;
	int resolution;
	int cross_sections;

	int vbo_start;
	int vbo_end;
	int ebo_start;
	int ebo_end;
	bt_aabb bounds;
} node;

vec3 get_line_point(struct line_t *l, float t);
vec3 get_line_end_point(struct line_t *l);
int get_line(node *n, float percent, float *offset);
float get_line_length(node *stem);

#endif /* NODE_H */

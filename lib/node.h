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

struct position_t {
	int index;
	float t;
};

typedef struct node_t {
	tm_vec3 glob_pos;
	int line_count;
	struct line_t *lines;

	int branch_count;
	int branch_capacity;
	float branch_density;
	struct node_t *branches;
	tm_vec3 *branch_curve;
	int branch_curve_size;

	int dichotomous_start;
	int terminal;

	float radius;
	float min_radius;
	tm_vec3 *radius_curve;
	int radius_curve_size;

	int depth;
	int resolution;
	int cross_sections;

	int vbo_start;
	int vbo_end;
	int ebo_start;
	int ebo_end;
	tm_aabb bounds;
} node;

node *new_nodes(int);
void remove_nodes(node *);
void expand_branches(node *);
vec3 get_line_point(struct line_t *l, float t);
vec3 get_line_end_point(struct line_t *l);
int get_line(node *n, float percent, float *offset);
float get_line_length(node *stem);

#endif /* NODE_H */

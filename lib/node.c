/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "node.h"

void reset_node(node *n)
{
	n->branch_count = 0;
	n->branch_curve_size = 0;
	n->radius_curve_size = 0;
	n->dichotomous_start = -1;
}

node *new_nodes(int count)
{
	node *n = malloc(sizeof(node)*count);
	for (count--; count >= 0; count--)
		reset_node(&n[count]);
	return n;
}

void remove_node(node *n)
{
	int i;
	if (n != NULL) {
		for (i = 0; i < n->branch_count; i++)
			remove_node(&n->branches[i]);

		if (n->branch_capacity > 0)
			free(n->branches);
		if (n->line_count > 0)
			free(n->lines);
	}
}

void remove_nodes(node *n)
{
	int i;
	for (i = 0; i < n->branch_count; i++) {
		remove_node(&n->branches[i]);
		reset_node(&n->branches[i]);
	}
	n->branch_count = 0;
	n->dichotomous_start = -1;
}

void expand_branches(node *n)
{
	int capacity = n->branch_capacity * 2;
	int size = capacity * sizeof(node);
	int i;

	n->branches = realloc(n->branches, size);
	for (i = n->branch_capacity; i < capacity; i++)
		reset_node(&n->branches[i]);
	n->branch_capacity = capacity;
}

vec3 get_line_point(struct line_t *l, float t)
{
	vec3 v;
	v.x = l->direction.x*t + l->start.x;
	v.y = l->direction.y*t + l->start.y;
	v.z = l->direction.z*t + l->start.z;
	return v;
}

vec3 get_line_end_point(struct line_t *l)
{
	return get_line_point(l, l->length);
}

float get_line_length(node *stem)
{
	float len = 0.0f;
	int i = 0;
	for (; i < stem->line_count; i++)
		len += stem->lines[i].length;
	return len;
}

int get_line(node *n, float percent, float *offset)
{
	float d = percent * get_line_length(n);
	float s = 0.0f;
	int j = 0;

	for (; j < n->cross_sections; j++) {
		s += n->lines[j].length;
		if (d <= s) {
			*offset = n->lines[j].length - (s - d);
			return j;
		}
	}
}

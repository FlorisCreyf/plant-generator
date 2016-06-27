/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "node.h"

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

/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "node.h"
#include <stdlib.h>

void resetNode(Node *n)
{
	n->branchCount = 0;
	n->branchCurveSize = 0;
	n->radiusCurveSize = 0;
	n->dichotomousStart = -1;
	n->lineCount = 0;
}

Node *newNodes(int count)
{
	Node *n = malloc(sizeof(Node)*count);
	for (count--; count >= 0; count--)
		resetNode(&n[count]);
	return n;
}

void removeNode(Node *n)
{
	int i;
	if (n != NULL) {
		for (i = 0; i < n->branchCount; i++)
			removeNode(&n->branches[i]);
		if (n->branchCapacity > 0)
			free(n->branches);
		if (n->lineCount > 0)
			free(n->lines);
	}
}

void removeNodes(Node *n)
{
	int i;
	for (i = 0; i < n->branchCount; i++) {
		removeNode(&n->branches[i]);
		resetNode(&n->branches[i]);
	}
	n->branchCount = 0;
	n->dichotomousStart = -1;
}

void expandBranches(Node *n)
{
	int capacity = n->branchCapacity * 2;
	int size = capacity * sizeof(Node);
	int i;

	n->branches = realloc(n->branches, size);
	for (i = n->branchCapacity; i < capacity; i++)
		resetNode(&n->branches[i]);
	n->branchCapacity = capacity;
}

TMvec3 getLinePoint(struct Line *l, float t)
{
	float x = l->direction.x*t + l->start.x;
	float y = l->direction.y*t + l->start.y;
	float z = l->direction.z*t + l->start.z;
	return (TMvec3){x, y, z};
}

TMvec3 getLineEndPoint(struct Line *l)
{
	return getLinePoint(l, l->length);
}

float getLineLength(Node *stem)
{
	float len = 0.0f;
	int i;
	for (i = 0; i < stem->lineCount; i++)
		len += stem->lines[i].length;
	return len;
}

int getLine(Node *n, float percent, float *offset)
{
	float d = percent * getLineLength(n);
	float s = 0.0f;
	int j;

	for (j = 0; j < n->crossSections; j++) {
		s += n->lines[j].length;
		if (d <= s) {
			*offset = n->lines[j].length - (s - d);
			return j;
		}
	}
}

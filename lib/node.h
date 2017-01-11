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

typedef struct Point {
	float x;
	float y;
} Point;

typedef struct Line {
	TMvec3 start;
	TMvec3 direction;
	float length;
} Line;

typedef struct Position {
	int index;
	float t;
} Position;

typedef struct Node {
	TMvec3 globPos;
	int lineCount;
	struct Line *lines;

	int branchCount;
	int branchCapacity;
	float branchDensity;
	struct Node *branches;
	TMvec3 *branchCurve;
	int branchCurveSize;

	int dichotomousStart;
	int terminal;

	float radius;
	float minRadius;
	TMvec3 *radiusCurve;
	int radiusCurveSize;

	int depth;
	int resolution;
	int crossSections;

	int vboStart;
	int vboEnd;
	int iboStart;
	int iboEnd;
	TMaabb bounds;
} Node;

Node *newNodes(int);
void removeNodes(Node *);
void expandBranches(Node *);
TMvec3 getLinePoint(struct Line *l, float t);
TMvec3 getLineEndPoint(struct Line *l);
int getLine(Node *n, float percent, float *offset);
float getLineLength(Node *stem);

#endif /* NODE_H */

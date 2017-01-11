/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "tree_struct.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

static int maxBranchDepth;
static TreeData *td;

struct Position getPathStartPosition(Node *n)
{
	struct Position pos = {-1, 0.0f};
	int i;

	for (i = 0; i < n->lineCount; i++) {
		float endY = getLineEndPoint(&n->lines[i]).y;

		if (endY >= td->crownBaseHeight) {
			pos.t = td->crownBaseHeight - n->lines[i].start.y;
			pos.index = i;
			break;
		}
	}

	return pos;
}

int incrementPosition(Node *n, struct Position *pos, float distance)
{
	distance += pos->t;
	while (distance >= n->lines[pos->index].length) {
		if (++pos->index >= n->lineCount)
			return 0;
		distance -= n->lines[pos->index-1].length;
	}
	pos->t = distance;
	return 1;
}

TMvec3 getBranchDirection(Node *n, Node *p)
{
	float ax;
	float ay;
	TMvec3 v = {0.0f, 1.0f, 0.0f};
	TMmat4 r;

	float ratio = n->radius/p->radius;
	ax = (float)rand() / RAND_MAX * ratio * 4.f;
	ay = (float)rand() / RAND_MAX;
	r = tmRotateXY(M_PI*0.1f + ax, ay*M_PI*2.0f);
	tmTransform(&v, &r, 0.0f);
	return v;
}

void getDichotomousDirections(Node *n, TMvec3 *d)
{
	float ax = (float)rand() / RAND_MAX / 2.0f;
	float ay = (float)rand() / (RAND_MAX) + M_PI*.25f;
	TMvec3 a1 = n->lines[n->lineCount-1].direction;
	TMvec3 a2 = {a1.x, 0, a1.z};
	TMvec3 b = tmCrossVec3(&a1, &a2);
	b = tmRotateAroundAxis(&b, &a1, ay);
	d[0] = tmRotateAroundAxis(&a1, &b, ax);
	d[1] = tmRotateAroundAxis(&a1, &b, -ax);
}

void setPath(Node *n, TMvec3 *start, TMvec3 *direction)
{
	int i;
	float len = (15*n->radius);

	n->lineCount = 3;
	n->lines = malloc(sizeof(struct Line) * n->lineCount);
	n->lines[0].start = (TMvec3){0.f, 0.f, 0.f};
	n->lines[0].length = len;
	n->lines[0].direction = *direction;
	n->globPos = *start;

	for (i = 1; i < n->lineCount; i++) {
		struct Line *l = &n->lines[i-1];
		n->lines[i].start = getLinePoint(l, l->length);
		n->lines[i].length = len;
		n->lines[i].direction = l->direction;
		n->lines[i].direction.x += 0.02;
		n->lines[i].direction.z -= 0.05;
	}
}

void setRadiusCurve(Node *n)
{
	if (n->radiusCurveSize > 0)
		free(n->radiusCurve);
	n->radiusCurve = malloc(sizeof(TMvec3)*8);
	n->radiusCurveSize = 8;
	n->radiusCurve[0] = (TMvec3){0.0f, -0.3f, 1.0f};
	n->radiusCurve[1] = (TMvec3){0.0f, -0.3f, 0.75f};
	n->radiusCurve[2] = (TMvec3){0.05f, -0.3f, 0.75f};
	n->radiusCurve[3] = (TMvec3){0.175f, -0.3f, 0.75f};
	n->radiusCurve[4] = (TMvec3){0.175f, -0.3f, 0.75f};
	n->radiusCurve[5] = (TMvec3){0.3f, -0.3f, 0.75f};
	n->radiusCurve[6] = (TMvec3){0.75f, -0.3f, 0.25f};
	n->radiusCurve[7] = (TMvec3){1.0f, -0.3f, 0.0f};
}

void setBranchCurve(Node *n)
{
	if (n->branchCurveSize > 0)
		n->branchCurve = realloc(n->branchCurve, sizeof(TMvec3)*4);
	else
		n->branchCurve = malloc(sizeof(TMvec3)*4);
	n->branchCurveSize = 4;
	n->branchCurve[0] = (TMvec3){0.0f, -0.3f, 1.0f};
	n->branchCurve[1] = (TMvec3){0.25f, -0.3f, 1.0f};
	n->branchCurve[2] = (TMvec3){0.75f, -0.3f, 1.0f};
	n->branchCurve[3] = (TMvec3){1.0f, -0.3f, 1.0f};
}

void addLateralBranches(Node *stem, struct Position pos)
{
	Node *n;
	int i;

	if (pos.index < 0 || stem->branchDensity <= 0)
		return;

	for (i = stem->branchCount; i < stem->branchCapacity; i++) {
		n = &stem->branches[i];
		n->radius = stem->radius * 0.35f / pow(i + 1.0f, 0.3f);
		n->crossSections = 4;
		n->resolution = stem->resolution-4 < 5 ? 5 : stem->resolution-4;
		n->dichotomousStart = -1;
		n->terminal = 0;
		n->branchDensity = 0.0f;
		TMvec3 start = getLinePoint(&stem->lines[pos.index], pos.t);
		start = tmAddVec3(&start, &stem->globPos);
		TMvec3 direction = getBranchDirection(n, stem);
		setPath(n, &start, &direction);
		setRadiusCurve(n);
		setBranchCurve(n);
		n->branchCapacity = 10;
		n->branchCount = 0;
		n->branches = newNodes(n->branchCapacity);
		n->depth = stem->depth + 1;

		if ((float)rand() / RAND_MAX > 0.8) {
			n->minRadius = 0.02f;
			addDichotomousBranches(n);
		} else
			n->minRadius = 0.01f;

		if (++stem->branchCount == stem->branchCapacity)
			expandBranches(stem);
		if (!incrementPosition(stem, &pos, 1.0f/stem->branchDensity))
			break;
	}
}

void setDichotomousBranch(Node *n, Node *p, TMvec3 direction)
{
	TMvec3 start = getLineEndPoint(&p->lines[p->lineCount-1]);
	start = tmAddVec3(&p->globPos, &start);
	n->radius = p->minRadius;
	n->minRadius = 0.01f;
	n->crossSections = 4;
	n->resolution = p->resolution;
	n->branchCapacity = 0;
	n->dichotomousStart = -1;
	n->terminal = 1;
	n->branchDensity = 0.0f;
	n->depth = p->depth + 1;
	setPath(n, &start, &direction);
	setRadiusCurve(n);
	setBranchCurve(n);
}

TMvec3 addDichotomousBranches(Node *stem)
{
	Node *n;
	TMvec3 d[2];

	if (stem->branchCount + 2 >= stem->branchCapacity)
		expandBranches(stem);

	getDichotomousDirections(stem, d);
	stem->dichotomousStart = stem->branchCount;
	n = &stem->branches[stem->branchCount++];
	setDichotomousBranch(n, stem, d[0]);
	n = &stem->branches[stem->branchCount++];
	setDichotomousBranch(n, stem, d[1]);
}

Node *newTreeStructure(TreeData *data, Node *root)
{
	td = data;
	root->branchCapacity = 20;
	root->branches = newNodes(root->branchCapacity);
	root->depth = 1;
	root->minRadius = 0.04f;
	root->terminal = 0;
	root->branchDensity = 2.0f;
	TMvec3 origin = {0.0f, 0.0f, 0.0f};
	TMvec3 direction = {0.0f, 1.0f, 0.0f};
	setPath(root, &origin, &direction);
	setRadiusCurve(root);
	setBranchCurve(root);

	addLateralBranches(root, getPathStartPosition(root));
	addDichotomousBranches(root);
}

void freeTreeStructure(Node *root)
{
	removeNodes(root);
}

void resetTreeStructure(Node *root)
{
	int i;
	for (i = 0; i < root->branchCount; i++)
		freeTreeStructure(&root->branches[i]);
	if (root->lineCount > 0)
		free(root->lines);
}

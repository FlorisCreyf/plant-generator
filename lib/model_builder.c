/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "model_builder.h"
#include "mesh_size.h"
#include "vector.h"
#include "curve.h"
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

static float *vbo;
static unsigned short *ibo;
static int vboSize;
static int iboSize;
static unsigned int vboIndex;
static unsigned short iboIndex;
static int overflow;

void addCSPoint(float *buffer, float radius, float angle, TMmat4 *t, int *i)
{
	TMvec3 normal = {cosf(angle), 0.0f, sinf(angle)};
	TMvec3 point = tmMultVec3(radius, &normal);

	tmTransform(&normal, t, 0.0f);
	tmTransform(&point, t, 1.0f);

	buffer[(*i)++] = point.x;
	buffer[(*i)++] = point.y;
	buffer[(*i)++] = point.z;
	buffer[(*i)++] = normal.x;
	buffer[(*i)++] = normal.y;
	buffer[(*i)++] = normal.z;
}

void makeCrossSection(float *buffer, TMmat4 *t, float radius, float res)
{
	const float rotation = 360.0f / res * M_PI / 180.0f;
	float angle = 0.0f;
	int i = 0;

	while (i < res*6) {
		addCSPoint(buffer, radius, angle, t, &i);
		angle += rotation;
	}
}

int addRect(unsigned short *buffer, int *l, int *h, int *i)
{
	buffer[(*i)++] = *h;
	buffer[(*i)++] = ++(*h);
	buffer[(*i)++] = *l;

	buffer[(*i)++] = *l;
	buffer[(*i)++] = *h;
	buffer[(*i)++] = ++(*l);
}

int addLastRect(unsigned short *buffer, int l, int h, int il, int ih, int *i)
{
	buffer[(*i)++] = ih;
	buffer[(*i)++] = il;
	buffer[(*i)++] = l;

	buffer[(*i)++] = l;
	buffer[(*i)++] = h;
	buffer[(*i)++] = ih;
}

int addTriangleRing(unsigned short *buffer, int l, int h, int res)
{
	int initH = h;
	int initL = l;
	int i = 0;
	while (i < 6 * (res-1))
		addRect(buffer, &l, &h, &i);
	addLastRect(buffer, l, h, initL, initH, &i);
	return i;
}

int addRectSeg(unsigned short *buffer, int l, int h, int s, int e, int res)
{
	int initH = h;
	int initL = l;
	int i = 0, j = 0;

	for (; j < res-1; j++)
		if (j >= s && j <= e)
			addRect(buffer, &l, &h, &i);
		else {
			l++;
			h++;
		}

	if (j >= s && j <= e) {
		l = initL + res - 1;
		h = initH + res - 1;
		addLastRect(buffer, l, h, initL, initH, &i);
	}

	return i;
}

TMmat4 getBranchTransform(struct Line *l, float offset, TMvec3 pos)
{
	float r1;
	float r2;
	TMvec3 normal = {0.0f, 1.0f, 0.0f};
	TMmat4 rotation = tmRotateIntoVec(&normal, &(l->direction));
	TMmat4 translation;
	TMmat4 transform;

	translation = tmTranslate(0.0f, offset, 0.0f);
	pos = tmAddVec3(&pos, &l->start);
	transform = tmTranslate(pos.x, pos.y, pos.z);
	transform = tmMultMat4(&transform, &rotation);
	return tmMultMat4(&transform, &translation);
}

float getRadius(Node *stem, float c)
{
	if (stem->radiusCurveSize < 2)
		c = stem->radius * pow(1.0f - c, 0.75f);
	else {
		int n = stem->radiusCurveSize / 4;
		c = stem->radius * tmGetPath(c, stem->radiusCurve, n).z;
	}
	return c > stem->minRadius ? c : stem->minRadius;
}

void addCrossSection(float *buffer, Node *stem, float percent)
{
	float offset;
	int j = getLine(stem, percent, &offset);
	TMmat4 t = getBranchTransform(&stem->lines[j], offset, stem->globPos);
	float r = getRadius(stem, percent);
	makeCrossSection(buffer, &t, r, stem->resolution);
}

void addCrossSectionO(float *buffer, Node *stem, Line *l, float offset)
{
	float len = getLineLength(stem);
	float p = (len - offset + l->length)/len;
	TMmat4 t = getBranchTransform(l, offset, stem->globPos);
	float r = getRadius(stem, p);
	makeCrossSection(buffer, &t, r, stem->resolution);
}

TMvec3 getCSNormal(int i)
{
	TMvec3 a = {vbo[i+3], vbo[i+4], vbo[i+5]};
	TMvec3 b = {vbo[i+9], vbo[i+10], vbo[i+11]};
	TMvec3 c = tmCrossVec3(&b, &a);
	tmNormalizeVec3(&c);
	return c;
}

float getBranchAngle(int a, TMvec3 dir)
{
	TMvec3 normA = getCSNormal(a);
	TMvec3 y = {0.f, 1.f, 0.f};
	TMmat4 rot = tmRotateIntoVec(&normA, &y);
	tmTransform(&dir, &rot, 0.0f);
	float angle = atan(dir.z/dir.x);
	if (dir.z < 0 && dir.x >= 0)
		angle += 2.f*M_PI;
	else if (dir.z < 0 && dir.x < 0)
		angle += M_PI;
	else if (dir.z >= 0 && dir.x < 0)
		angle += M_PI;
	return angle;
}

void getBounds(TMvec3 dir, int t, int a, int *lb, int *ub, float *angle)
{
	*angle = getBranchAngle(a, dir);
	float inc = 2.f*M_PI/t;
	int min = *lb = *ub = (int)round(*angle/inc);
	int i;

	for (i = 1; i < (t)/2; i++) {
		float a = absf(*angle - (*lb - 1.f)*inc - inc/2.f);
		float b = absf(*angle - (*ub + 1.f)*inc - inc/2.f);
		if (a < b)
			(*lb)--;
		else
			(*ub)++;
	}

	*lb = *lb < 0 ? *lb+t : *lb;
	*ub = *ub < t ? *ub : *ub-t;
}

/** This limits the max number of vertices per cross section to 180. */
int isDichotomousTwisted(float angle, int t)
{
	int m = angle*180.f/M_PI - 90;
	int l = 360.f/t;
	return m%l >= l/2;
}

/** Connects three cross sections labeled a, b, and c. */
void connectDichotomous(int a, int b, int c, int t, Node *n)
{
	int ub, lb;
	float angle;
	int twisted;
	int x, y;
	int i;
	getBounds(n[1].lines[0].direction, t, a, &lb, &ub, &angle);
	twisted = isDichotomousTwisted(angle, t);

	a /= 6;
	b /= 6;
	c /= 6;

	if (lb > ub)
		iboIndex += addRectSeg(&ibo[iboIndex], a, b, ub+1, lb-1, t);
	else {
		iboIndex += addRectSeg(&ibo[iboIndex], a, b, 0, lb-1, t);
		iboIndex += addRectSeg(&ibo[iboIndex], a, b, ub+1, t, t);
	}

	if (lb < ub)
		iboIndex += addRectSeg(&ibo[iboIndex], a, c, lb, ub, t);
	else {
		iboIndex += addRectSeg(&ibo[iboIndex], a, c, 0, ub, t);
		iboIndex += addRectSeg(&ibo[iboIndex], a, c, lb, t, t);
	}

	x = ub+1 >= t ? ub+1-t : ub+1;
	ibo[iboIndex++] = b + x;
	ibo[iboIndex++] = a + x;
	ibo[iboIndex++] = c + x;
	y = x;
	if (twisted && t%2 != 0)
		y = y+1 < t ? y+1 : 0;
	for (i = 0; i < t/2 + (twisted || t%2 == 0 ? 0 : 1); i++) {
		if (twisted) {
			ibo[iboIndex++] = b + (x-1 < 0 ? t-1 : x-1);
			ibo[iboIndex++] = b + x;
			ibo[iboIndex++] = c + y;
			y = y+1 < t ? y+1 : 0;
			x = x-1 < 0 ? t-1 : x-1;
		} else {
			ibo[iboIndex++] = c + x;
			ibo[iboIndex++] = c + (x+1 < t ? x+1 : 0);
			ibo[iboIndex++] = b + y;
			y = y-1 < 0 ? t-1 : y-1;
			x = x+1 < t ? x+1 : 0;
		}
	}

	ibo[iboIndex++] = a + lb;
	ibo[iboIndex++] = b + lb;
	ibo[iboIndex++] = c + lb;
	x = y = lb;
	if (!twisted && t%2 != 0)
		y = y-1 < 0 ? t-1 : y-1;
	for (i = 0; i < t/2 + (!twisted || t%2 == 0 ? 0 : 1); i++) {
		if (twisted) {
			ibo[iboIndex++] = c + (x-1 < 0 ? t-1 : x-1);
			ibo[iboIndex++] = c + x;
			ibo[iboIndex++] = b + y;
			y = y+1 < t ? y+1 : 0;
			x = x-1 < 0 ? t-1 : x-1;
		} else {
			ibo[iboIndex++] = b + x;
			ibo[iboIndex++] = b + (x+1 < t ? x+1 : 0);
			ibo[iboIndex++] = c + y;
			y = y-1 < 0 ? t-1 : y-1;
			x = x+1 < t ? x+1 : 0;
		}
	}
}

int addBranch(Node *stem, Node *parent, float offset);

void addDichotomous(Node *n, Node *p, int aIndex)
{
	struct Line *l = &p->lines[p->lineCount-1];
	float offset = l->length - n->radius;
	int bIndex = vboIndex;

	addCrossSectionO(&vbo[aIndex], p, l, offset);
	if (!addBranch(&n[0], p, 0.05)) {
		overflow = 1;
		return;
	}

	if (getDichotomousICount(&n[0]) + iboIndex > iboSize) {
		overflow = 1;
		return;
	}
	connectDichotomous(aIndex, bIndex, vboIndex, p->resolution, n);

	if(!addBranch(&n[1], p, 0.05)) {
		overflow = 1;
		return;
	}
}

void addSubBranches(Node *stem, int prevIndex)
{
	int i = 0;

	for (; i < stem->branchCount; i++) {
		if (i == stem->dichotomousStart) {
			addDichotomous(&stem->branches[i], stem, prevIndex);
			if (overflow)
				return;
			i++;
		} else
			if (!addBranch(&stem->branches[i], stem, 0.0f)) {
				overflow = 1;
				return;
			}
	}
}

void capBranch(int vertex, int t)
{
	int i;
	for (i = 0; i < t/2-1; i++) {
		ibo[iboIndex++] = vertex + i;
		ibo[iboIndex++] = vertex + t - i - 1;
		ibo[iboIndex++] = vertex + i + 1;

		ibo[iboIndex++] = vertex + i + 1;
		ibo[iboIndex++] = vertex + t - i - 1;
		ibo[iboIndex++] = vertex + t - i - 2;
	}

	if (t&1 != 0) {
		ibo[iboIndex++] = vertex + i;
		ibo[iboIndex++] = vertex + i + 2;
		ibo[iboIndex++] = vertex + i + 1;
	}
}

float offsetToPercent(Node *stem, float offset, int i)
{
	float len = getLineLength(stem);
	float a = (len - offset) / (stem->crossSections-1);
	return (offset + i*a) / len;
}

void setBranchBounds(Node *stem)
{
	int len = stem->vboEnd - stem->vboStart;
	stem->bounds = tmCreateAABB(&vbo[stem->vboStart], len);
}

int addBranch(Node *stem, Node *parent, float offset)
{
	unsigned short lIndex;
	unsigned short rIndex;
	int prevIndex;
	int i, j;
	float p;

	if (stem == NULL)
		return 1;

	stem->vboStart = vboIndex;
	stem->iboStart = iboIndex;

	for (i = 0; i < stem->crossSections - 1; i++) {
		if (getBranchVCount(stem) + vboIndex > vboSize)
			return 0;
		else if (getBranchICount(stem) + iboIndex > iboSize)
			return 0;

		p = offsetToPercent(stem, offset, i);
		addCrossSection(&vbo[vboIndex], stem, p);
		prevIndex = vboIndex;
		vboIndex += getBranchVCount(stem);
		addTriangleRing(&ibo[iboIndex], prevIndex/6, vboIndex/6,
				stem->resolution);
		iboIndex += getBranchICount(stem);
	}

	if (getBranchVCount(stem) + vboIndex > vboSize)
		return 0;

	prevIndex = vboIndex;
	vboIndex += getBranchVCount(stem);
	stem->vboEnd = vboIndex;
	stem->iboEnd = iboIndex;

	addSubBranches(stem, prevIndex);
	if (overflow)
		return 0;

	if (stem->dichotomousStart == -1) {
		if (getCapICount(stem) + iboIndex > iboSize)
			return 0;

		addCrossSection(&vbo[prevIndex], stem, 1.0f);
		capBranch(prevIndex/6, stem->resolution);
	}

	setBranchBounds(stem);

	return 1;
}

int buildModel(float *v, int vSize, unsigned short *i, int iSize, Node *root)
{
	vbo = v;
	ibo = i;
	vboSize = vSize;
	iboSize = iSize;
	vboIndex = 0;
	iboIndex = 0;
	overflow = 0;

	if(!addBranch(root, NULL, 0.0f))
		return 0;
	else
		return 1;
}

int getVBOSize()
{
	return vboIndex/6;
}

int getIBOSize()
{
	return iboIndex;
}

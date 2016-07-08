/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "primitives.h"

void addVec3(int &i, float *v, float x, float y, float z)
{
	v[i++] = x;
	v[i++] = y;
	v[i++] = z;
}

Line createGrid(int sections)
{
	Line grid;
	float *v;
	const int vertices = 8 * sections + 4;
	const int size = vertices * 3 * 2;
	int i = 0;

	grid.vertices.resize(size);
	grid.attribs = 2;
	grid.stride = sizeof(float) * 3;
	grid.width = 1.5f;
	grid.program = 0;
	v = &grid.vertices[0];

	addVec3(i, v, 0.0f, 0.0f, sections);
	addVec3(i, v, 0.41f, 0.41f, 0.41f);
	addVec3(i, v, 0.0f, 0.0f, -sections);
	addVec3(i, v, 0.41f, 0.41f, 0.41f);
	addVec3(i, v, sections, 0.0f, 0.0f);
	addVec3(i, v, 0.41f, 0.41f, 0.41f);
	addVec3(i, v, -sections, 0.0f, 0.0f);
	addVec3(i, v, 0.41f, 0.41f, 0.41f);

	for (int j = 1; j <= sections; j++) {
		addVec3(i, v, j, 0.0f, sections);
		addVec3(i, v, 0.46f, 0.46f, 0.46f);
		addVec3(i, v, j, 0.0f, -sections);
		addVec3(i, v, 0.46f, 0.46f, 0.46f);
		addVec3(i, v, -j, 0.0f, sections);
		addVec3(i, v, 0.46f, 0.46f, 0.46f);
		addVec3(i, v, -j, 0.0f, -sections);
		addVec3(i, v, 0.46f, 0.46f, 0.46f);

		addVec3(i, v, sections, 0.0f, j);
		addVec3(i, v, 0.46f, 0.46f, 0.46f);
		addVec3(i, v, -sections, 0.0f, j);
		addVec3(i, v, 0.46f, 0.46f, 0.46f);
		addVec3(i, v, sections, 0.0f, -j);
		addVec3(i, v, 0.46f, 0.46f, 0.46f);
		addVec3(i, v, -sections, 0.0f, -j);
		addVec3(i, v, 0.46f, 0.46f, 0.46f);
	}

	return grid;
}

void addVec3c(int &i, float *v, float x, float y, float z)
{
	addVec3(i, v, x, y, z);
	addVec3(i, v, 0.4f, 0.4f, 0.4f);
}

Line createBox(bt_aabb &b)
{
	Line l;
	float *v;
	const int vertices = 24;
	const int size = vertices * 3 * 2;
	int i = 0;

	l.vertices.resize(size);
	l.attribs = 2;
	l.stride = sizeof(float) * 3;
	l.width = 2.0f;
	l.program = 0;
	v = &l.vertices[0];

	addVec3c(i, v, b.x1, b.y1, b.z1); addVec3c(i, v, b.x1, b.y2, b.z1);
	addVec3c(i, v, b.x1, b.y1, b.z1); addVec3c(i, v, b.x1, b.y1, b.z2);
	addVec3c(i, v, b.x1, b.y1, b.z1); addVec3c(i, v, b.x2, b.y1, b.z1);
	addVec3c(i, v, b.x2, b.y2, b.z2); addVec3c(i, v, b.x1, b.y2, b.z2);
	addVec3c(i, v, b.x2, b.y2, b.z2); addVec3c(i, v, b.x2, b.y2, b.z1);
	addVec3c(i, v, b.x2, b.y2, b.z2); addVec3c(i, v, b.x2, b.y1, b.z2);
	addVec3c(i, v, b.x1, b.y2, b.z1); addVec3c(i, v, b.x1, b.y2, b.z2);
	addVec3c(i, v, b.x2, b.y1, b.z1); addVec3c(i, v, b.x2, b.y1, b.z2);
	addVec3c(i, v, b.x1, b.y1, b.z2); addVec3c(i, v, b.x2, b.y1, b.z2);
	addVec3c(i, v, b.x2, b.y1, b.z1); addVec3c(i, v, b.x2, b.y2, b.z1);
	addVec3c(i, v, b.x1, b.y1, b.z2); addVec3c(i, v, b.x1, b.y2, b.z2);
	addVec3c(i, v, b.x1, b.y2, b.z1); addVec3c(i, v, b.x2, b.y2, b.z1);

	return l;
}

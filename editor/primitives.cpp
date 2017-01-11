/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "primitives.h"
#include "curve.h"

void addVec3(float *&v, float x, float y, float z)
{
	*(v++) = x;
	*(v++) = y;
	*(v++) = z;
}

void addVec3(float *&v, TMvec3 a)
{
	*(v++) = a.x;
	*(v++) = a.y;
	*(v++) = a.z;
}

void addPoint(float *&v, float x, float y, float z, TMmat4 *t, TMvec3 color)
{
	TMvec3 point = {x, y, z};
	tmTransform(&point, t, 1.0f);
	addVec3(v, point);
	addVec3(v, color);
}

void addLine(float *&v, TMvec3 a, TMvec3 b, TMvec3 color)
{
	addVec3(v, a);
	addVec3(v, color);
	addVec3(v, b);
	addVec3(v, color);
}

float *setGeometry(GeometryComponent &g, int size)
{
	g.attribs = 2;
	g.stride = sizeof(float) * 6;
	g.vertices.resize(g.vertices.size() + size);
	return &g.vertices[g.vertices.size() - size];
}

void createGrid(GeometryComponent &g, int sections, TMvec3 color,
                TMvec3 sectionColor, TMmat4 transform)
{
	float *v = setGeometry(g, (8*sections + 4) * 6);

	addPoint(v, 0.0f, 0.0f, sections, &transform, sectionColor);
	addPoint(v, 0.0f, 0.0f, -sections, &transform, sectionColor);
	addPoint(v, sections, 0.0f, 0.0f, &transform, sectionColor);
	addPoint(v, -sections, 0.0f, 0.0f, &transform, sectionColor);

	for (int j = 1; j <= sections; j++) {
		addPoint(v, j, 0.0f, sections, &transform, color);
		addPoint(v, j, 0.0f, -sections, &transform, color);
		addPoint(v, -j, 0.0f, sections, &transform, color);
		addPoint(v, -j, 0.0f, -sections, &transform, color);

		addPoint(v, sections, 0.0f, j, &transform, color);
		addPoint(v, -sections, 0.0f, j, &transform, color);
		addPoint(v, sections, 0.0f, -j, &transform, color);
		addPoint(v, -sections, 0.0f, -j, &transform, color);
	}
}

void createBox(GeometryComponent &g, TMaabb &b, TMvec3 c)
{
	float *v = setGeometry(g, 144);
	addLine(v, (TMvec3){b.x1, b.y1, b.z1}, (TMvec3){b.x2, b.y1, b.z1}, c);
	addLine(v, (TMvec3){b.x1, b.y1, b.z1}, (TMvec3){b.x1, b.y2, b.z1}, c);
	addLine(v, (TMvec3){b.x1, b.y1, b.z1}, (TMvec3){b.x1, b.y1, b.z2}, c);
	addLine(v, (TMvec3){b.x2, b.y2, b.z2}, (TMvec3){b.x1, b.y2, b.z2}, c);
	addLine(v, (TMvec3){b.x2, b.y2, b.z2}, (TMvec3){b.x2, b.y1, b.z2}, c);
	addLine(v, (TMvec3){b.x2, b.y2, b.z2}, (TMvec3){b.x2, b.y2, b.z1}, c);
	addLine(v, (TMvec3){b.x2, b.y1, b.z1}, (TMvec3){b.x2, b.y2, b.z1}, c);
	addLine(v, (TMvec3){b.x2, b.y1, b.z1}, (TMvec3){b.x2, b.y1, b.z2}, c);
	addLine(v, (TMvec3){b.x1, b.y2, b.z1}, (TMvec3){b.x2, b.y2, b.z1}, c);
	addLine(v, (TMvec3){b.x1, b.y2, b.z1}, (TMvec3){b.x1, b.y2, b.z2}, c);
	addLine(v, (TMvec3){b.x1, b.y1, b.z2}, (TMvec3){b.x2, b.y1, b.z2}, c);
	addLine(v, (TMvec3){b.x1, b.y1, b.z2}, (TMvec3){b.x1, b.y2, b.z2}, c);
}

void createLine(GeometryComponent &g, vector<TMvec3> p, TMvec3 color)
{
	float *v = setGeometry(g, 6*p.size());
	for (int i = p.size() - 1; i >= 0; --i) {
		addVec3(v, p[i].x, p[i].y, p[i].z);
		addVec3(v, color);
	}
}

void createBezier(GeometryComponent &g, vector<TMvec3> p, int resolution,
		TMvec3 color)
{
	float *v = setGeometry(g, resolution*6);

	for (int i = 0; i < resolution; i++) {
		float r = i/(float)(resolution-1);
		addVec3(v, tmGetBezier(r, &p[0], p.size()));
		addVec3(v, color);
	}
}

void createPath(GeometryComponent &g, vector<TMvec3> p, int resolution,
                TMvec3 color)
{
	int curves = p.size()/4;
	float *v = setGeometry(g, resolution*6*curves);

	for (int i = 0; i < curves; i++)
		for (int j = 0; j < resolution; j++) {
			float r = j/(float)(resolution-1);
			addVec3(v, tmGetBezier(r, &p[i*4], 4));
			addVec3(v, color);
		}
}

void createPlane(GeometryComponent &g, TMvec3 a, TMvec3 b, TMvec3 c)
{
	const int size = 36;
	float *v = setGeometry(g, size);
	TMvec3 d = tmAddVec3(&a, &b);

	addVec3(v, c);
	addVec3(v, .32f, 0.32f, 0.32f);
	addVec3(v, tmAddVec3(&c, &a));
	addVec3(v, .32f, 0.32f, 0.32f);

	addVec3(v, tmAddVec3(&c, &b));
	addVec3(v, .32f, 0.32f, 0.32f);
	addVec3(v, tmAddVec3(&c, &d));
	addVec3(v, .32f, 0.32f, 0.32f);

	int start = (g.vertices.size() - size)/6;
	g.triangles.push_back(start);
	g.triangles.push_back(start + 1);
	g.triangles.push_back(start + 2);
	g.triangles.push_back(start + 2);
	g.triangles.push_back(start + 1);
	g.triangles.push_back(start + 3);
}

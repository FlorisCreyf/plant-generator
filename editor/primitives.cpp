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

void addVec3(float *&v, bt_vec3 a)
{
	*(v++) = a.x;
	*(v++) = a.y;
	*(v++) = a.z;
}

void addPoint(float *&v, float x, float y, float z, bt_mat4 *t, bt_vec3 color)
{
	bt_vec3 point = {x, y, z};
	bt_transform(&point, t, 1.0f);
	addVec3(v, point);
	addVec3(v, color);
}

void addLine(float *&v, bt_vec3 a, bt_vec3 b, bt_vec3 color)
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

void createGrid(GeometryComponent &g, int sections, bt_vec3 color,
                bt_vec3 sectionColor, bt_mat4 transform)
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

void createBox(GeometryComponent &g, bt_aabb &b, bt_vec3 c)
{
	float *v = setGeometry(g, 144);
	addLine(v, (bt_vec3){b.x1, b.y1, b.z1}, (bt_vec3){b.x2, b.y1, b.z1}, c);
	addLine(v, (bt_vec3){b.x1, b.y1, b.z1}, (bt_vec3){b.x1, b.y2, b.z1}, c);
	addLine(v, (bt_vec3){b.x1, b.y1, b.z1}, (bt_vec3){b.x1, b.y1, b.z2}, c);
	addLine(v, (bt_vec3){b.x2, b.y2, b.z2}, (bt_vec3){b.x1, b.y2, b.z2}, c);
	addLine(v, (bt_vec3){b.x2, b.y2, b.z2}, (bt_vec3){b.x2, b.y1, b.z2}, c);
	addLine(v, (bt_vec3){b.x2, b.y2, b.z2}, (bt_vec3){b.x2, b.y2, b.z1}, c);
	addLine(v, (bt_vec3){b.x2, b.y1, b.z1}, (bt_vec3){b.x2, b.y2, b.z1}, c);
	addLine(v, (bt_vec3){b.x2, b.y1, b.z1}, (bt_vec3){b.x2, b.y1, b.z2}, c);
	addLine(v, (bt_vec3){b.x1, b.y2, b.z1}, (bt_vec3){b.x2, b.y2, b.z1}, c);
	addLine(v, (bt_vec3){b.x1, b.y2, b.z1}, (bt_vec3){b.x1, b.y2, b.z2}, c);
	addLine(v, (bt_vec3){b.x1, b.y1, b.z2}, (bt_vec3){b.x2, b.y1, b.z2}, c);
	addLine(v, (bt_vec3){b.x1, b.y1, b.z2}, (bt_vec3){b.x1, b.y2, b.z2}, c);
}

void createLine(GeometryComponent &g, std::vector<bt_vec3> p, bt_vec3 color)
{
	float *v = setGeometry(g, 6*p.size());
	for (int i = p.size() - 1; i >= 0; --i) {
		addVec3(v, p[i].x, p[i].y, p[i].z);
		addVec3(v, color);
	}
}

void createBezier(GeometryComponent &g, std::vector<bt_vec3> p, int resolution,
		bt_vec3 color)
{
	float *v = setGeometry(g, resolution*6);

	for (int i = 0; i < resolution; i++) {
		float r = i/(float)(resolution-1);
		addVec3(v, bt_get_bezier(r, &p[0], p.size()));
		addVec3(v, color);
	}
}

void createPath(GeometryComponent &g, std::vector<bt_vec3> p, int resolution,
                bt_vec3 color)
{
	int curves = p.size()/4;
	float *v = setGeometry(g, resolution*6*curves);

	for (int i = 0; i < curves; i++)
		for (int j = 0; j < resolution; j++) {
			float r = j/(float)(resolution-1);
			addVec3(v, bt_get_bezier(r, &p[i*4], 4));
			addVec3(v, color);
		}
}

void createPlane(GeometryComponent &g, bt_vec3 a, bt_vec3 b, bt_vec3 c)
{
	const int size = 36;
	float *v = setGeometry(g, size);
	bt_vec3 d = bt_add_vec3(&a, &b);

	addVec3(v, c);
	addVec3(v, .32f, 0.32f, 0.32f);
	addVec3(v, bt_add_vec3(&c, &a));
	addVec3(v, .32f, 0.32f, 0.32f);

	addVec3(v, bt_add_vec3(&c, &b));
	addVec3(v, .32f, 0.32f, 0.32f);
	addVec3(v, bt_add_vec3(&c, &d));
	addVec3(v, .32f, 0.32f, 0.32f);

	int start = (g.vertices.size() - size)/6;
	g.triangles.push_back(start);
	g.triangles.push_back(start + 1);
	g.triangles.push_back(start + 2);
	g.triangles.push_back(start + 2);
	g.triangles.push_back(start + 1);
	g.triangles.push_back(start + 3);
}

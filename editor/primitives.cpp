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

void addLine(float *&v, bt_vec3 a, bt_vec3 b, bt_vec3 color)
{
	addVec3(v, a.x, a.y, a.z);
	addVec3(v, color.r, color.g, color.b);
	addVec3(v, b.x, b.y, b.z);
	addVec3(v, color.r, color.g, color.b);
}

void createGrid(GeometryComponent &g, int sections, float scale)
{
	float *v;
	const int size = (8*sections + 4) * 3*2;
	g.attribs = 2;
	g.stride = sizeof(float) * 6;
	g.vertices.resize(g.vertices.size() + size);
	v = &g.vertices[g.vertices.size() - size];

	addVec3(v, 0.0f, 0.0f, sections*scale);
	addVec3(v, 0.41f, 0.41f, 0.41f);
	addVec3(v, 0.0f, 0.0f, -sections*scale);
	addVec3(v, 0.41f, 0.41f, 0.41f);
	addVec3(v, sections*scale, 0.0f, 0.0f);
	addVec3(v, 0.41f, 0.41f, 0.41f);
	addVec3(v, -sections*scale, 0.0f, 0.0f);
	addVec3(v, 0.41f, 0.41f, 0.41f);

	for (int j = 1; j <= sections; j++) {
		addVec3(v, j*scale, 0.0f, sections*scale);
		addVec3(v, 0.46f, 0.46f, 0.46f);
		addVec3(v, j*scale, 0.0f, -sections*scale);
		addVec3(v, 0.46f, 0.46f, 0.46f);
		addVec3(v, -j*scale, 0.0f, sections*scale);
		addVec3(v, 0.46f, 0.46f, 0.46f);
		addVec3(v, -j*scale, 0.0f, -sections*scale);
		addVec3(v, 0.46f, 0.46f, 0.46f);

		addVec3(v, sections*scale, 0.0f, j*scale);
		addVec3(v, 0.46f, 0.46f, 0.46f);
		addVec3(v, -sections*scale, 0.0f, j*scale);
		addVec3(v, 0.46f, 0.46f, 0.46f);
		addVec3(v, sections*scale, 0.0f, -j*scale);
		addVec3(v, 0.46f, 0.46f, 0.46f);
		addVec3(v, -sections*scale, 0.0f, -j*scale);
		addVec3(v, 0.46f, 0.46f, 0.46f);
	}
}

void createBox(GeometryComponent &g, bt_aabb &b)
{
	float *v;
	const int size = 144;
	bt_vec3 c = {0.4f, 0.4f, 0.4f};
	g.attribs = 2;
	g.stride = sizeof(float) * 6;
	g.vertices.resize(g.vertices.size() + size);
	v = &g.vertices[g.vertices.size() - size];

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

void createLine(GeometryComponent &g, std::vector<bt_vec3> p)
{
	float *v;
	const int size = 6 * p.size();
	g.attribs = 2;
	g.stride = sizeof(float) * 6;
	g.vertices.resize(g.vertices.size() + size);
	v = &g.vertices[g.vertices.size() - size];

	for (int i = p.size() - 1; i >= 0; --i) {
		addVec3(v, p[i].x, p[i].y, p[i].z);
		addVec3(v, .2f, 0.46f, 0.6f);
	}
}

void createBezier(GeometryComponent &g, std::vector<bt_vec3> p, int resolution)
{
	float *v;
	const int size = 100 * 6;
	g.vertices.resize(g.vertices.size() + size);
	g.attribs = 2;
	g.stride = sizeof(float) * 6;
	v = &g.vertices[g.vertices.size() - size];

	for (int i = 0; i < 100; i += 2) {
		bt_vec3 color = {0.5f, 0.5f, 0.5f};
		bt_vec3 a = bt_get_bezier(i/99.f, &p[0], p.size());
		bt_vec3 b = bt_get_bezier((i+1)/99.f, &p[0], p.size());
		addLine(v, a, b, color);
	}
}

void createPlane(GeometryComponent &g, bt_vec3 a, bt_vec3 b)
{
	float *v;
	const int size = 36;

	g.vertices.resize(g.vertices.size() + size);
	g.attribs = 2;
	g.stride = sizeof(float) * 6;
	v = &g.vertices[g.vertices.size() - size];

	addVec3(v, a.x + b.x, a.y + b.y, a.z + b.z);
	addVec3(v, .32f, 0.32f, 0.32f);
	addVec3(v, a.x - b.x, a.y - b.y, a.z - b.z);
	addVec3(v, .32f, 0.32f, 0.32f);

	addVec3(v, -a.x + b.x, -a.y + b.y, -a.z + b.z);
	addVec3(v, .32f, 0.32f, 0.32f);
	addVec3(v, -a.x - b.x, -a.y - b.y, -a.z - b.z);
	addVec3(v, .32f, 0.32f, 0.32f);

	int start = (g.vertices.size() - size)/6;
	g.triangles.push_back(start);
	g.triangles.push_back(start + 1);
	g.triangles.push_back(start + 2);
	g.triangles.push_back(start + 2);
	g.triangles.push_back(start + 1);
	g.triangles.push_back(start + 3);
}

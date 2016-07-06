/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "scene.h"
#include "collision.h"
#include <cstdio>

void Scene::add(Mesh m)
{
	meshes.push_back(m);
}

void Scene::add(Line l)
{
	lines.push_back(l);
}

Mesh *Scene::getMesh(int i)
{
	return &meshes[i];
}

int Scene::getSelected()
{
	return selected;
}

int Scene::setSelected(Camera &camera, int x, int y)
{
	bt_vec3 dir = camera.getRayDirection(x, y);
	bt_vec3 orig = camera.getPosition();
	selected = getId(orig, dir);
	return selected;
}

int Scene::getId(bt_vec3 origin, bt_vec3 direction)
{
	bt_aabb box;
	float t;
	int len;

	for (int i = 0; i < (int)meshes.size(); i++) {
		len = meshes[i].vusage * 12;
		box = bt_create_aabb(&meshes[i].vertices[0], len);
		t = bt_intersects_aabb(origin, direction, box);
		if (t != 0.0f)
			return i;
	}

	return -1;
}

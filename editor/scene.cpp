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

Scene::Scene()
{
	selected = selectedBranch = -1;
}

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

int Scene::getSelectedBranch()
{
	return selectedBranch;
}

int Scene::setSelected(Camera &camera, int x, int y)
{
	bt_vec3 direction = camera.getRayDirection(x, y);
	bt_vec3 origin = camera.getPosition();
	bt_aabb box;
	float t;
	int len;

	for (int i = 0; i < (int)meshes.size(); i++) {
		len = meshes[i].vusage * 12;
		box = bt_create_aabb(&meshes[i].vertices[0], len);
		t = bt_intersects_aabb(origin, direction, box);
		if (t != 0.0f) {
			selected = i;
			return selected;
		}
	}

	selected = -1;
	return selected;
}

int Scene::setSelectedBranch(Camera &camera, int x, int y, bt_tree tree)
{
	bt_vec3 dir = camera.getRayDirection(x, y);
	bt_vec3 orig = camera.getPosition();
	bt_aabb box = bt_get_bounding_box(tree, 0);
	float t;

	for (int i = 0; box.x1 != box.x2;) {
		t = bt_intersects_aabb(orig, dir, box);
		if (t != 0.0f) {
			selectedBranch = i;
			return i;
		}
		box = bt_get_bounding_box(tree, ++i);
	}

	selectedBranch = -1;
	return selectedBranch;
}

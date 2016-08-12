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
	selected = NULL;
	selectedBranch = -1;
}

void Scene::add(Entity e)
{
	entities.push_back(e);
}

Entity *Scene::getEntity(int i)
{
	return &entities[i];
}

Entity *Scene::getSelected()
{
	return selected;
}

int Scene::getSelectedBranch()
{
	return selectedBranch;
}

Entity *Scene::setSelected(Camera &camera, int x, int y)
{
	bt_vec3 direction = camera.getRayDirection(x, y);
	bt_vec3 origin = camera.getPosition();

	for (int i = 0; i < (int)entities.size(); i++) {
		GeometryComponent *g = &entities[i].geometry;
		for (int j = 0; j < entities[i].renderComponent.size(); j++) {
			RenderComponent *r = &entities[i].renderComponent[j];
			int len = (r->vertexRange[1] - r->vertexRange[0]) * 12;
			bt_aabb box = bt_create_aabb(&g->vertices[0], len);
			float t = bt_intersects_aabb(origin, direction, box);

			if (t != 0.0f) {
				selected = &entities[i];
				return selected;
			}
		}
	}

	selected = NULL;
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

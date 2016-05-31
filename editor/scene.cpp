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

Mesh Scene::getMesh(int i)
{
	return meshes[i];
}

void Scene::get(bt_vec3 origin, bt_vec3 direction)
{

}

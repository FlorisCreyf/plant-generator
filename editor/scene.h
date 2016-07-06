/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef SCENE_H
#define SCENE_H

#include "objects.h"
#include "render_system.h"
#include "camera.h"
#include "vector.h"
#include <vector>

class Scene
{
public:
	void add(Mesh m);
	void add(Line l);
	int getSelected();
	int setSelected(Camera &camera, int x, int y);
	Mesh *getMesh(int i);

private:
	std::vector<Mesh> meshes;
	std::vector<Line> lines;
	int selected;
	int selectionProgram;

	int getId(bt_vec3 origin, bt_vec3 direction);
};

#endif /* SCENE_H */

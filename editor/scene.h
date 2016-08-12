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
#include "camera.h"
#include "vector.h"
#include "bluetree.h"
#include <vector>

class Scene
{
public:
	Scene();
	void add(Entity e);
	Entity *getSelected();
	int getSelectedBranch();
	Entity *setSelected(Camera &camera, int x, int y);
	int setSelectedBranch(Camera &camera, int x, int y, bt_tree tree);
	Entity *getEntity(int i);

private:
	std::vector<Entity> entities;
	Entity *selected;
	int selectedBranch;
	int selectionProgram;
};

#endif /* SCENE_H */

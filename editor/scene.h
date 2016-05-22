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

#include "vector.h"
#include <vector>

class Scene
{
public:
	Scene();
	int newObject();
	void createVBO(int id, int size);
	void createEBO(int id, int size);
	void addAttrib(int id, int index, int size, int stride, int start);
	void getObject(int w, int h, int x, int y, bt_mat4 inv);

private:
	int count;
	struct object {
		int id;
		float *vb;
		float *pb;
		unsigned short *eb;
		bool visible;
	};
	std::vector<object> objects;

	bt_vec3 getRayDirection(int w, int h, int x, int y, bt_mat4 inv);

};

#endif /* SCENE_H */

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

class Scene
{
public:
	void findMesh(int w, int h, int x, int y, bt_mat4 inv);	

private:
	bt_vec3 getRayDirection(int w, int h, int x, int y, bt_mat4 inv);

};

#endif /* SCENE_H */

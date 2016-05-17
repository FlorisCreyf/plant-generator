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

void findMesh(int w, int h, int x, int y, bt_mat4 inv)
{

}

bt_vec3 Scene::getRayDirection(int w, int h, int x, int y, bt_mat4 inv)
{
	bt_vec3 p;
	float hm;

	p.x = (2.0f * x) / w - 1.0f;
	p.y = (2.0f * y) / h - 1.0f;
	p.z = 0.0f;

	hm = bt_transform(&p, &inv, 0.0f);

	return bt_mult_vec3(1.0f / hm, &p);;
}

/* 
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef CAMERA_H
#define CAMERA_H

#include "vector.h"

class Camera
{
public:
	Camera();

	void setStartCoordinates(float x, float y);
	void setCoordinates(float x, float y);
	void setPerspective(float fovy, float near, float far, float aspect);
	bt_mat4 getCrystalBallMatrix();

private:

	struct point {
		float x;
		float y;
	} startPosition, position;
	bt_mat4 perspective;

	bt_mat4 getLookAtMatrix(bt_vec3 eye, bt_vec3 target, bt_vec3 up);
};

#endif /* CAMERA_H */


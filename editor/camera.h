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
	void setPan(float x, float y);
	void setCoordinates(float x, float y);
	void setPerspective(float fovy, float near, float far, float aspect);
	void setWindowSize(int width, int height);
	void zoom(float x, float y);
	bt_vec3 getPosition();
	bt_mat4 getVP();
	bt_vec3 getRayDirection(int x, int y);

	enum Action {
		ZOOM, ROTATE, PAN, NONE
	} action;

private:
	struct Point {
		float x;
		float y;
	};
	Point posDiff;
	Point pos;
	Point start;

	bt_mat4 perspective;
	bt_vec3 up;
	bt_vec3 eye;
	bt_vec3 feye;
	bt_vec3 target;
	bt_vec3 ftarget;
	int winWidth;
	int winHeight;
	float distance;
	float fdistance;

	bt_vec3 getCameraPosition();
	bt_mat4 getInverseVP();
	bt_mat4 getInversePerspective();
	bt_mat4 getInverseLookAt();
	bt_mat4 getLookAtMatrix(bt_vec3 *eye, bt_vec3 *target, bt_vec3 *up);
};

#endif /* CAMERA_H */

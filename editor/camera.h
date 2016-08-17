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
	tm_vec3 getPosition();
	tm_mat4 getVP();
	tm_vec3 getRayDirection(int x, int y);

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

	tm_mat4 perspective;
	tm_vec3 up;
	tm_vec3 eye;
	tm_vec3 feye;
	tm_vec3 target;
	tm_vec3 ftarget;
	int winWidth;
	int winHeight;
	float distance;
	float fdistance;

	tm_vec3 getCameraPosition();
	tm_mat4 getInverseVP();
	tm_mat4 getInversePerspective();
	tm_mat4 getInverseLookAt();
	tm_mat4 getLookAtMatrix(tm_vec3 *eye, tm_vec3 *target, tm_vec3 *up);
};

#endif /* CAMERA_H */

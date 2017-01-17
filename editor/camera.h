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

class Camera {
public:
	Camera();
	void setStartCoordinates(float x, float y);
	void setPan(float x, float y);
	void setCoordinates(float x, float y);
	void setPerspective(float fovy, float near, float far, float aspect);
	void setWindowSize(int width, int height);
	void zoom(float y);
	TMvec3 getPosition();
	TMmat4 getVP();
	TMvec3 getRayDirection(int x, int y);

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

	TMmat4 perspective;
	TMvec3 up;
	TMvec3 eye;
	TMvec3 feye;
	TMvec3 target;
	TMvec3 ftarget;
	int winWidth;
	int winHeight;
	float distance;
	float fdistance;

	TMvec3 getCameraPosition();
	TMmat4 getInverseVP();
	TMmat4 getInversePerspective();
	TMmat4 getInverseLookAt();
	TMmat4 getLookAtMatrix(TMvec3 *eye, TMvec3 *target, TMvec3 *up);
};

#endif /* CAMERA_H */

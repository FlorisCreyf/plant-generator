/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "camera.h"
#include <cmath>
#include <stdio.h>

#define UNUSED(x) (void)(x)

Camera::Camera()
{
	up = {0.0f, 1.0f, 0.0f};
	posDiff = {0.0f, 0.0f};
	pos = {0.0f, 10.0f};
	ftarget = target = {0.0f, 4.0f, 0.0f};
	fdistance = distance = 15.0f;
}

void Camera::setStartCoordinates(float x, float y)
{
	posDiff.x = x - (pos.x - posDiff.x);
	posDiff.y = y - (pos.y - posDiff.y);
	start.y = y;
	start.x = x;
	pos = {x, y};
	fdistance = distance;
	ftarget = target;
}

void Camera::setCoordinates(float x, float y)
{
	pos = {x, y};
}

void Camera::setPan(float x, float y)
{
	TMvec3 a, b, f;
	TMvec3 eye = getCameraPosition();
	TMvec3 dir = tmSubVec3(&eye, &target);
	tmNormalizeVec3(&dir);
	float panSpeed = distance * 0.002f;

	a = tmCrossVec3(&dir, &up);
	b = tmCrossVec3(&dir, &a);

	tmNormalizeVec3(&a);
	tmNormalizeVec3(&b);

	a = tmMultVec3((x - start.x)*panSpeed, &a);
	b = tmMultVec3((start.y - y)*panSpeed, &b);
	f = tmAddVec3(&a, &b);
	target = tmAddVec3(&f, &ftarget);
}

void Camera::setWindowSize(int width, int height)
{
	winWidth = width;
	winHeight = height;
}

void Camera::zoom(float y)
{
	float b = (y - start.y) / 10.0f;
	if (fdistance + b > 0.1f)
		distance = fdistance + b;
}

TMvec3 Camera::getCameraPosition()
{
	const float toRadian = M_PI / 180.0f;
	float x = (pos.x - posDiff.x + 0.1f) * toRadian * 0.5f;
	float y = (pos.y - posDiff.y + 0.1f) * toRadian * 0.5f;
	eye.x = distance * cos(x) * cos(y) + target.x;
	eye.y = distance * sin(y) + target.y;
	eye.z = distance * sin(x) * cos(y) + target.z;

	if (std::abs(x) >= M_PI*2.0f)
		posDiff.x = pos.x;
	if (std::abs(y) >= M_PI*2.0f)
		posDiff.y = pos.y;
	else if (std::abs(y) < M_PI*0.5f)
		up = {0.0f, 1.0f, 0.0f};
	else if (std::abs(y) < M_PI*1.5f)
		up = {0.0f, -1.0f, 0.0f};
	else if (std::abs(y) >= M_PI*1.5f)
		up = {0.0f, 1.0f, 0.0f};
	else if (std::abs(y) >= M_PI*0.5f)
		up = {0.0f, -1.0f, 0.0f};

	return eye;
}

TMvec3 Camera::getPosition()
{
	return eye;
}

TMmat4 Camera::getVP()
{
	TMvec3 eye = getCameraPosition();
	TMmat4 l = getLookAtMatrix(&eye, &target, &up);
	return tmMultMat4(&perspective, &l);
}

TMmat4 Camera::getInverseVP()
{
	TMmat4 invP = getInversePerspective();
	TMmat4 invL = getInverseLookAt();
	return tmMultMat4(&invL, &invP);
}

TMmat4 Camera::getLookAtMatrix(TMvec3 *eye, TMvec3 *center, TMvec3 *up)
{
	float x, y, z;
	TMvec3 a, b, c;

	a = tmSubVec3(center, eye);
	tmNormalizeVec3(&a);
	b = tmCrossVec3(&a, up);
	tmNormalizeVec3(&b);
	c = tmCrossVec3(&b, &a);

	x = -tmDotVec3(&b, eye);
	y = -tmDotVec3(&c, eye);
	z = tmDotVec3(&a, eye);

	return (TMmat4){
		b.x, c.x, -a.x, 0.0f,
		b.y, c.y, -a.y, 0.0f,
		b.z, c.z, -a.z, 0.0f,
		x, y, z, 1.0f
	};
}

TMmat4 Camera::getInverseLookAt()
{
	TMvec3 eye = getCameraPosition();
	TMmat4 a = getLookAtMatrix(&eye, &target, &up);
	TMmat4 b = {0.0f};

	b.m[3][3] = 1.0f;
	for (int i = 0; i < 3;i++)
		for (int j = 0; j < 3; j++)
			b.m[i][j] = a.m[j][i];

	TMvec3 t = {a.m[3][0], a.m[3][1], a.m[3][2]};
	tmTransform(&t, &b, 0.0f);
	b.m[3][0] = -t.x;
	b.m[3][1] = -t.y;
	b.m[3][2] = -t.z;

	return b;
}

void Camera::setPerspective(float fovy, float near, float far, float aspect)
{
	float t = tan((fovy * M_PI / 180.0f) / 2.0f);
	float a = 1.0f / (aspect * t);
	float b = 1.0f / t;
	float c = -(far + near) / (far - near);
	float d = -1.0f;
	float e = -(2.0f * far * near) / (far - near);

	perspective = (TMmat4){
		a, 0.0f, 0.0f, 0.0f,
		0.0f, b, 0.0f, 0.0f,
		0.0f, 0.0f, c, d,
		0.0f, 0.0f, e, 0.0f
	};
}

TMmat4 Camera::getInversePerspective()
{
	float a = perspective.m[0][0];
	float b = perspective.m[1][1];
	float c = perspective.m[2][2];
	float d = perspective.m[2][3];
	float e = perspective.m[3][2];

	return (TMmat4){
		1.0f/a, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f/b, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f/e,
		0.0f, 0.0f, 1.0f/d, -c/(e*d)
	};
}

TMvec3 Camera::getRayDirection(int x, int y)
{
	TMvec3 p;
	TMmat4 invP = getInversePerspective();
	TMmat4 invL = getInverseLookAt();

	p.x = 2.0f*x/(winWidth-1) - 1.0f;
	p.y = 1.0f - 2.0f*y/(winHeight-1);
	p.z = -1.0f;
	tmTransform(&p, &invP, 1.0f);
	p.z = -1.f;
	tmTransform(&p, &invL, 0.0f);
	tmNormalizeVec3(&p);

	return p;
}

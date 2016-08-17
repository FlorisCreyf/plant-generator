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
	vec3 a, b, f;
	vec3 eye = getCameraPosition();
	vec3 dir = tm_sub_vec3(&eye, &target);
	tm_normalize_vec3(&dir);
	float panSpeed = distance * 0.002f;

	a = tm_cross_vec3(&dir, &up);
	b = tm_cross_vec3(&dir, &a);

	tm_normalize_vec3(&a);
	tm_normalize_vec3(&b);

	a = tm_mult_vec3((x - start.x)*panSpeed, &a);
	b = tm_mult_vec3((start.y - y)*panSpeed, &b);
	f = tm_add_vec3(&a, &b);
	target = tm_add_vec3(&f, &ftarget);
}

void Camera::setWindowSize(int width, int height)
{
	winWidth = width;
	winHeight = height;
}

void Camera::zoom(float x, float y)
{
	float b = (y - start.y) / 10.0f;
	if (fdistance + b > 0.1f)
		distance = fdistance + b;
}

tm_vec3 Camera::getCameraPosition()
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

tm_vec3 Camera::getPosition()
{
	return eye;
}

tm_mat4 Camera::getVP()
{
	tm_vec3 eye = getCameraPosition();
	tm_mat4 l = getLookAtMatrix(&eye, &target, &up);
	return tm_mult_mat4(&perspective, &l);
}

tm_mat4 Camera::getInverseVP()
{
	tm_mat4 invP = getInversePerspective();
	tm_mat4 invL = getInverseLookAt();
	return tm_mult_mat4(&invL, &invP);
}

tm_mat4 Camera::getLookAtMatrix(tm_vec3 *eye, tm_vec3 *center, tm_vec3 *up)
{
	float x, y, z;
	tm_vec3 a, b, c;

	a = tm_sub_vec3(center, eye);
	tm_normalize_vec3(&a);
	b = tm_cross_vec3(&a, up);
	tm_normalize_vec3(&b);
	c = tm_cross_vec3(&b, &a);

	x = -tm_dot_vec3(&b, eye);
	y = -tm_dot_vec3(&c, eye);
	z = tm_dot_vec3(&a, eye);

	return (tm_mat4){
		b.x, c.x, -a.x, 0.0f,
		b.y, c.y, -a.y, 0.0f,
		b.z, c.z, -a.z, 0.0f,
		x, y, z, 1.0f
	};
}

tm_mat4 Camera::getInverseLookAt()
{
	tm_vec3 eye = getCameraPosition();
	tm_mat4 a = getLookAtMatrix(&eye, &target, &up);
	tm_mat4 b = {0.0f};

	b.m[3][3] = 1.0f;
	for (int i = 0; i < 3;i++)
		for (int j = 0; j < 3; j++)
			b.m[i][j] = a.m[j][i];

	tm_vec3 t = {a.m[3][0], a.m[3][1], a.m[3][2]};
	tm_transform(&t, &b, 0.0f);
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

	perspective = (tm_mat4){
		a, 0.0f, 0.0f, 0.0f,
		0.0f, b, 0.0f, 0.0f,
		0.0f, 0.0f, c, d,
		0.0f, 0.0f, e, 0.0f
	};
}

tm_mat4 Camera::getInversePerspective()
{
	float a = perspective.m[0][0];
	float b = perspective.m[1][1];
	float c = perspective.m[2][2];
	float d = perspective.m[2][3];
	float e = perspective.m[3][2];

	return (tm_mat4){
		1.0f/a, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f/b, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f/e,
		0.0f, 0.0f, 1.0f/d, -c/(e*d)
	};
}

tm_vec3 Camera::getRayDirection(int x, int y)
{
	tm_vec3 p;
	tm_mat4 invP = getInversePerspective();
	tm_mat4 invL = getInverseLookAt();

	p.x = 2.0f*x/(winWidth-1) - 1.0f;
	p.y = 1.0f - 2.0f*y/(winHeight-1);
	p.z = -1.0f;
	tm_transform(&p, &invP, 1.0f);
	p.z = -1.f;
	tm_transform(&p, &invL, 0.0f);
	tm_normalize_vec3(&p);

	return p;
}

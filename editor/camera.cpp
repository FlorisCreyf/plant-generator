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
	pos = {0.0f, 0.0f};
	ftarget = target = {0.0f, 2.0f, 0.0f};
	fdistance = distance = 10.0f;
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
	vec3 dir = bt_sub_vec3(&eye, &target);
	bt_normalize_vec3(&dir);
	float panSpeed = distance * 0.002f;

	a = bt_cross_vec3(&dir, &up);
	b = bt_cross_vec3(&dir, &a);

	bt_normalize_vec3(&a);
	bt_normalize_vec3(&b);

	a = bt_mult_vec3((x - start.x)*panSpeed, &a);
	b = bt_mult_vec3((start.y - y)*panSpeed, &b);
	f = bt_add_vec3(&a, &b);
	target = bt_add_vec3(&f, &ftarget);
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

bt_vec3 Camera::getCameraPosition()
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

bt_vec3 Camera::getPosition()
{
	return eye;
}

bt_mat4 Camera::getVP()
{
	bt_vec3 eye = getCameraPosition();
	bt_mat4 l = getLookAtMatrix(&eye, &target, &up);
	return bt_mult_mat4(&perspective, &l);
}

bt_mat4 Camera::getInverseVP()
{
	bt_mat4 invP = getInversePerspective();
	bt_mat4 invL = getInverseLookAt();
	return bt_mult_mat4(&invL, &invP);
}

bt_mat4 Camera::getLookAtMatrix(bt_vec3 *eye, bt_vec3 *center, bt_vec3 *up)
{
	float x, y, z;
	bt_vec3 a, b, c;

	a = bt_sub_vec3(center, eye);
	bt_normalize_vec3(&a);
	b = bt_cross_vec3(&a, up);
	bt_normalize_vec3(&b);
	c = bt_cross_vec3(&b, &a);

	x = -bt_dot_vec3(&b, eye);
	y = -bt_dot_vec3(&c, eye);
	z = bt_dot_vec3(&a, eye);

	return (bt_mat4){
		b.x, c.x, -a.x, 0.0f,
		b.y, c.y, -a.y, 0.0f,
		b.z, c.z, -a.z, 0.0f,
		x, y, z, 1.0f
	};
}

bt_mat4 Camera::getInverseLookAt()
{
	bt_vec3 eye = getCameraPosition();
	bt_mat4 a = getLookAtMatrix(&eye, &target, &up);
	bt_mat4 b = {0.0f};

	b.m[3][3] = 1.0f;
	for (int i = 0; i < 3;i++)
		for (int j = 0; j < 3; j++)
			b.m[i][j] = a.m[j][i];

	bt_vec3 t = {a.m[3][0], a.m[3][1], a.m[3][2]};
	bt_transform(&t, &b, 0.0f);
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

	perspective = (bt_mat4){
		a, 0.0f, 0.0f, 0.0f,
		0.0f, b, 0.0f, 0.0f,
		0.0f, 0.0f, c, d,
		0.0f, 0.0f, e, 0.0f
	};
}

bt_mat4 Camera::getInversePerspective()
{
	float a = perspective.m[0][0];
	float b = perspective.m[1][1];
	float c = perspective.m[2][2];
	float d = perspective.m[2][3];
	float e = perspective.m[3][2];

	return (bt_mat4){
		1.0f/a, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f/b, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f/e,
		0.0f, 0.0f, 1.0f/d, -c/(e*d)
	};
}

bt_vec3 Camera::getRayDirection(int x, int y)
{
	bt_vec3 p;
	bt_mat4 inv = getInverseVP();

	p.x = 2.0f*x/winWidth - 1.0f;
	p.y = 1.0f - 2.0f*y/winHeight;
	p.z = 1.0f;

	bt_transform(&p, &inv, 1.0f);
	bt_normalize_vec3(&p);

	return p;
}

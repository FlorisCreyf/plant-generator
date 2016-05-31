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
	startPosition = (Point){0.0f, 0.0f};
	position = (Point){0.0f, 0.0f};
}

void Camera::setStartCoordinates(float x, float y)
{
	startPosition.x = x - (position.x - startPosition.x);
	startPosition.y = y - (position.y - startPosition.y);
	position = (Point){x, y};
}

void Camera::setCoordinates(float x, float y)
{
	position = (Point){x, y};
}

bt_vec3 Camera::getCameraPosition()
{
	const float toRadian = M_PI / 180.0f;
	float distance = 10.0f;
	float x = (position.x - startPosition.x) * toRadian;
	float y = (position.y - startPosition.y) * toRadian;
	cameraPos.x = distance * cos(x) * cos(y);
	cameraPos.y = distance * sin(y);
	cameraPos.z = distance * sin(x) * cos(y);
	return cameraPos;
}

bt_vec3 Camera::getPosition()
{
	return cameraPos;
}

bt_mat4 Camera::getVP()
{
	bt_mat4 l;
	bt_vec3 eye = getCameraPosition();
	bt_vec3 target = (bt_vec3){0.0f, 0.0f, 0.0f};
	bt_vec3 up = (bt_vec3){0.0f, 1.0f, 0.0f};

	eye.y += 2.0f;
	target.y += 2.0f;

	l = getLookAtMatrix(&eye, &target, &up);

	return bt_mult_mat4(&perspective, &l);
}

bt_mat4 Camera::getInverseVP()
{
	bt_mat4 invP = getInversePerspective();
	bt_mat4 invL;
	bt_vec3 eye = getCameraPosition();
	bt_vec3 target = (bt_vec3){0.0f, 0.0f, 0.0f};
	bt_vec3 up = (bt_vec3){0.0f, 1.0f, 0.0f};
	
	eye.y += 2.0f;
	target.y += 2.0f;

	invL = getInverseLookAt(&eye, &target, &up);

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

/** 
 * View = Rotation * Translation
 * View_inv = [R_inv * e1 | R_inv * e2 | R_inv * e2 | R_inv * T3_inv]
 */
bt_mat4 Camera::getInverseLookAt(bt_vec3 *eye, bt_vec3 *center, bt_vec3 *up)
{
	bt_mat4 a = getLookAtMatrix(eye, center, up);
	bt_mat4 b = (bt_mat4){0.0f};
	
	b.m[3][3] = 1.0f;
	for (int i = 0; i < 3;i++)
		for (int j = 0; j < 3; j++)
			b.m[i][j] = a.m[j][i];

	bt_vec3 t = (bt_vec3){a.m[3][0], a.m[3][1], a.m[3][2]};
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

bt_vec3 Camera::getRayDirection(int w, int h, int x, int y)
{
	bt_vec3 p;
	bt_mat4 inv = getInverseVP();
	float homog;

	p.x = 2.0f*x/w - 1.0f;
	p.y = 2.0f*y/h - 1.0f;
	p.z = 0.0f;
	homog = bt_transform(&p, &inv, 0.0f);

	return bt_mult_vec3(1.0f/homog, &p);;
}


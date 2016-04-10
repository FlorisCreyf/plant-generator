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

Camera::Camera()
{
	startPosition = (point){0.0f, 0.0f};
	position = (point){0.0f, 0.0f};
}

void Camera::setStartCoordinates(float x, float y)
{
	startPosition.x = x - (position.x - startPosition.x);
	startPosition.y = y - (position.y - startPosition.y);
	position = startPosition;
}

void Camera::setCoordinates(float x, float y)
{
	position = (point){x, y};
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
		a,    0.0f, 0.0f, 0.0f,
		0.0f, b,    0.0f, 0.0f,
		0.0f, 0.0f, c,    d,
		0.0f, 0.0f, e,    0.0f
	};
}

bt_mat4 Camera::getCrystalBallMatrix()
{
	const float TO_RADIAN = M_PI / 180.0f;

	float distance = 10.0f;
	float x = (position.x - startPosition.x) * TO_RADIAN;
	float y = (position.y - startPosition.y) * TO_RADIAN;

	float cameraX = distance * cos(x) * cos(y);
	float cameraY = distance * sin(y);
	float cameraZ = distance * sin(x) * cos(y);

	bt_vec3 eye = (bt_vec3){cameraX, cameraY, cameraZ};
	bt_vec3 target = (bt_vec3){0.0f, 0.0f, 0.0f};
	bt_vec3 up = (bt_vec3){0.0f, 1.0f, 0.0f};

	bt_mat4 l = getLookAtMatrix(eye, target, up);

	return bt_mult_mat4(&perspective, &l);
}

bt_mat4 Camera::getLookAtMatrix(bt_vec3 eye, bt_vec3 center, bt_vec3 up)
{
	bt_mat4 l;

	return l;
}


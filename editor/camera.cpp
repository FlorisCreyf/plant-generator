/* Plant Genererator
 * Copyright (C) 2016-2018  Floris Creyf
 *
 * Plant Genererator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Plant Genererator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "camera.h"
#include <cmath>
#include <stdio.h>

using pg::Vec3;
using pg::Vec4;
using pg::Mat4;

Camera::Camera()
{
	action = None;
	up = {0.0f, 1.0f, 0.0f};
	posDiff = {0.0f, 0.0f};
	pos = {0.0f, 30.0f};
	ftarget = target = {0.0f, 4.0f, 0.0f};
	fdistance = distance = 15.0f;
	panSpeed = 0.002f;
	zoomSpeed = 0.1f;
	zoomMin = 0.1f;
	zoomMax = 100.0f;
}

void Camera::setPanSpeed(float speed)
{
	panSpeed = speed;
}

void Camera::setZoom(float speed, float min, float max)
{
	zoomSpeed = speed;
	zoomMin = min;
	zoomMax = max;
}

void Camera::setOrientation(float x, float y)
{
	pos.x = x;
	pos.y = y;
}

void Camera::setTarget(pg::Vec3 target)
{
	this->target = target;
}

void Camera::setDistance(float distance)
{
	this->distance = distance;
}

void Camera::setAction(Action action)
{
	this->action = action;
}

void Camera::executeAction(float x, float y)
{
	switch (action) {
	case Zoom:
		zoom(y);
		break;
	case Rotate:
		setCoordinates(x, y);
		break;
	case Pan:
		setPan(x, y);
		break;
	default:
		break;
	}
}

void Camera::setStartCoordinates(float x, float y)
{
	/* posDiff becomes large if the viewport is rotated only in one
	 * direction. */
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
	Vec3 eye = getCameraPosition();
	Vec3 dir = pg::normalize(eye - target);
	float speed = distance * panSpeed;

	const float toRadian = M_PI / 360.0f;
	float xx = (pos.x - posDiff.x) * toRadian;
	pg::Vec3 c = {-std::sin(xx), 0.0f, std::cos(xx)};
	pg::Vec3 d = pg::cross(dir, c);

	c = (x - start.x) * speed * c;
	d = (start.y - y) * speed * d;

	target = c + d + ftarget;
}

void Camera::setWindowSize(int width, int height)
{
	winWidth = width;
	winHeight = height;
}

void Camera::zoom(float y)
{
	float b = (y - start.y) * zoomSpeed;
	if (fdistance + b > zoomMin && fdistance + b < zoomMax)
		distance = fdistance + b;
	else if (fdistance + b <= zoomMin)
		distance = zoomMin;
	else
		distance = zoomMax;
}

Vec3 Camera::getCameraPosition()
{
	const float toRadian = M_PI / 180.0f;
	float x = (pos.x - posDiff.x) * toRadian * 0.5f;
	float y = (pos.y - posDiff.y) * toRadian * 0.5f;
	eye.x = distance * std::cos(x) * std::cos(y) + target.x;
	eye.y = distance * std::sin(y) + target.y;
	eye.z = distance * std::sin(x) * std::cos(y) + target.z;

	if (!perspective)
		initOrthographic(near, far);

	if (std::abs(y) < M_PI*0.5f)
		up = {0.0f, 1.0f, 0.0f};
	else if (std::abs(y) < M_PI*1.5f)
		up = {0.0f, -1.0f, 0.0f};
	else if (std::abs(y) >= M_PI*1.5f)
		up = {0.0f, 1.0f, 0.0f};
	else if (std::abs(y) >= M_PI*0.5f)
		up = {0.0f, -1.0f, 0.0f};

	return eye;
}

Vec3 Camera::getPosition()
{
	return eye;
}

Vec3 Camera::getDirection()
{
	return pg::normalize(target - eye);
}

pg::Vec3 Camera::getTarget()
{
	return target;
}

Mat4 Camera::getVP()
{
	Vec3 eye = getCameraPosition();
	Mat4 l = getLookAtMatrix(eye, target);
	return projection * l;
}

Mat4 Camera::getInverseVP()
{
	Mat4 invP = getInversePerspective();
	Mat4 invL = getInverseLookAt();
	return invL * invP;
}

Mat4 Camera::getLookAtMatrix(Vec3 &eye, Vec3 &center)
{
	float x, y, z;
	float xx = (pos.x - posDiff.x) * M_PI / 360.0f;
	Vec3 a = pg::normalize(center - eye);
	Vec3 b = {std::sin(xx), 0.0f, -std::cos(xx)};
	Vec3 c = pg::cross(b, a);
	x = -pg::dot(b, eye);
	y = -pg::dot(c, eye);
	z = pg::dot(a, eye);

	return (Mat4){
		b.x, c.x, -a.x, 0.0f,
		b.y, c.y, -a.y, 0.0f,
		b.z, c.z, -a.z, 0.0f,
		x, y, z, 1.0f
	};
}

Mat4 Camera::getInverseLookAt()
{
	Vec3 eye = getCameraPosition();
	Mat4 a = getLookAtMatrix(eye, target);
	Mat4 b = pg::identity();

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			b[i][j] = a[j][i];

	Vec3 t = {a[3][0], a[3][1], a[3][2]};
	t = pg::toVec3(b * pg::toVec4(t, 0.0f));
	b[3][0] = -t.x;
	b[3][1] = -t.y;
	b[3][2] = -t.z;

	return b;
}

void Camera::initOrthographic(pg::Vec3 near, pg::Vec3 far)
{
	near.x *= distance;
	near.y *= distance;
	far.x *= distance;
	far.y *= distance;
	float a = -(far.x+near.x)/(far.x-near.x);
	float b = -(far.y+near.y)/(far.y-near.y);
	float c = -(far.z+near.z)/(far.z-near.z);

	projection = (Mat4){
		2.0f/(far.x-near.x), 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f/(far.y-near.y), 0.0f, 0.0f,
		0.0f, 0.0f, -2.0f/(far.z-near.z), 0.0f,
		a, b, c, 1.0f
	};
}

void Camera::setOrthographic(Vec3 near, Vec3 far)
{
	perspective = false;
	this->far = far;
	this->near = near;
	initOrthographic(near, far);
}

Mat4 Camera::getInverseOrthographic()
{
	float a = projection[0][0];
	float b = projection[1][1];
	float c = projection[2][2];
	float d = projection[3][0];
	float e = projection[3][1];
	float f = projection[3][2];

	return (Mat4){
		1.0f/a, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f/b, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f/c, 0.0f,
		-d/a, -e/b, -f/c, 1.0f
	};
}

void Camera::setPerspective(float fovy, float near, float far, float aspect)
{
	perspective = true;
	float t = std::tan((fovy * M_PI / 180.0f) / 2.0f);
	float a = 1.0f / (aspect * t);
	float b = 1.0f / t;
	float c = -(far + near) / (far - near);
	float d = -1.0f;
	float e = -(2.0f * far * near) / (far - near);

	projection = (Mat4){
		a, 0.0f, 0.0f, 0.0f,
		0.0f, b, 0.0f, 0.0f,
		0.0f, 0.0f, c, d,
		0.0f, 0.0f, e, 0.0f
	};
}

Mat4 Camera::getInversePerspective()
{
	float a = projection[0][0];
	float b = projection[1][1];
	float c = projection[2][2];
	float d = projection[2][3];
	float e = projection[3][2];

	return (Mat4){
		1.0f/a, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f/b, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f/e,
		0.0f, 0.0f, 1.0f/d, -c/(e*d)
	};
}

Vec3 Camera::toScreenSpace(Vec3 point)
{
	Mat4 vp = getVP();
	Vec4 v = vp * pg::toVec4(point, 1.0f);
	point = pg::toVec3(v);
	point.x /= v.w;
	point.y /= v.w;
	point.x = (point.x + 1.0f) / 2.0f * winWidth;
	point.y = winHeight - (point.y + 1.0f) / 2.0f * winHeight;
	return point;
}

pg::Ray Camera::getRay(int x, int y)
{
	if (perspective)
		return getPerspectiveRay(x, y);
	else
		return getOrthographicRay(x, y);
}

/**
 * The direction of the ray remains constant while the camera remains
 * stationary. The origin of the ray depends on the screen coordinates.
 */
pg::Ray Camera::getOrthographicRay(int x, int y)
{
	pg::Ray ray;
	Mat4 invProj = getInverseOrthographic();
	Mat4 invLook = getInverseLookAt();
	Vec3 p;

	p.x = 2.0f*x/(winWidth-1) - 1.0f;
	p.y = 1.0f - 2.0f*y/(winHeight-1);
	p.z = 0.0f;
	p = pg::toVec3(invProj * pg::toVec4(p, 1.0f));
	p.z = eye.z;
	p = pg::toVec3(invLook * pg::toVec4(p, 1.0f));

	ray.direction = getDirection();
	ray.origin = p;

	return ray;
}

/**
 * The direction of the ray depends on the screen coordinates. The origin of the
 * ray remains stationary at the camera position.
 */
pg::Ray Camera::getPerspectiveRay(int x, int y)
{
	pg::Ray ray;
	Mat4 invProj = getInversePerspective();
	Mat4 invLook = getInverseLookAt();
	Vec3 p;

	p.x = 2.0f*x/(winWidth-1) - 1.0f;
	p.y = 1.0f - 2.0f*y/(winHeight-1);
	p.z = -1.0f;
	p = pg::toVec3(invProj * pg::toVec4(p, 1.0f));
	p.z = -1.0f;
	p = pg::toVec3(invLook * pg::toVec4(p, 0.0f));
	p = pg::normalize(p);

	ray.direction = p;
	ray.origin = eye;
	return ray;
}

/* Plant Generator
 * Copyright (C) 2016-2018  Floris Creyf
 *
 * Plant Generator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Plant Generator is distributed in the hope that it will be useful,
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

using pg::Vec2;
using pg::Vec3;
using pg::Vec4;
using pg::Mat4;

const float pi = std::acos(-1.0);

Camera::Camera()
{
	this->action = Action::None;
	this->posDiff = Vec2(0.0f, 0.0f);
	this->pos = Vec2(180.0f, 30.0f);
	this->ftarget = this->target = Vec3(0.0f, 4.0f, 0.0f);
	this->distance = 18.0f;
	this->panSpeed = 0.002f;
	this->zoomSpeed = 0.01f;
	this->zoomMin = 0.1f;
	this->zoomMax = 100.0f;
	this->scroll = 0.0f;
	this->prevY = 0.0f;
}

void Camera::setPanSpeed(float speed)
{
	this->panSpeed = speed;
}

void Camera::setZoom(float speed, float min, float max)
{
	this->zoomSpeed = speed;
	this->zoomMin = min;
	this->zoomMax = max;
}

void Camera::setOrientation(float x, float y)
{
	this->pos.x = x;
	this->pos.y = y;
}

void Camera::setTarget(Vec3 target)
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

bool Camera::executeAction(float x, float y)
{
	if (this->action == Zoom)  {
		zoom(prevY - y);
		prevY = y;
		return true;
	} else if (this->action == Rotate) {
		setCoordinates(x, y);
		return true;
	} else if (this->action == Pan) {
		setPan(x, y);
		return true;
	}
	return false;
}

void Camera::setStartCoordinates(float x, float y)
{
	this->posDiff.x = x - (this->pos.x - this->posDiff.x);
	this->posDiff.y = y - (this->pos.y - this->posDiff.y);
	this->start.y = y;
	this->start.x = x;
	this->pos = Vec2(x, y);
	this->ftarget = this->target;
	this->prevY = this->pos.y;
}

void Camera::setCoordinates(float x, float y)
{
	this->pos = Vec2(x, y);
}

void Camera::setPan(float x, float y)
{
	Vec3 eye = getCameraPosition();
	Vec3 dir = pg::normalize(eye - target);
	float speed = distance * panSpeed;

	const float toRadian = pi / 360.0f;
	float xx = (this->pos.x - this->posDiff.x) * toRadian;
	Vec3 c(-std::sin(xx), 0.0f, std::cos(xx));
	Vec3 d = pg::cross(dir, c);

	c = (x - this->start.x) * speed * c;
	d = (this->start.y - y) * speed * d;

	this->target = c + d + this->ftarget;
}

void Camera::setWindowSize(int width, int height)
{
	this->winWidth = width;
	this->winHeight = height;
}

void Camera::zoom(float y)
{
	float newDistance = this->distance - y * 0.01f * sqrt(this->distance);
	if (newDistance > this->zoomMin && newDistance < this->zoomMax)
		this->distance = newDistance;
	else if (newDistance <= this->zoomMin)
		this->distance = this->zoomMin;
	else
		this->distance = this->zoomMax;
}

Vec3 Camera::getCameraPosition() const
{
	const float toRadian = pi / 180.0f;
	float x = (this->pos.x - this->posDiff.x) * toRadian * 0.5f;
	float y = (this->pos.y - this->posDiff.y) * toRadian * 0.5f;
	Vec3 eye;
	eye.x = this->distance * std::cos(x) * std::cos(y) + this->target.x;
	eye.y = this->distance * std::sin(y) + this->target.y;
	eye.z = this->distance * std::sin(x) * std::cos(y) + this->target.z;
	return eye;
}

Vec3 Camera::getPosition() const
{
	return this->eye;
}

Vec3 Camera::getDirection() const
{
	return pg::normalize(this->target - this->eye);
}

Vec3 Camera::getTarget() const
{
	return this->target;
}

Mat4 Camera::getVP() const
{
	Vec3 eye = getCameraPosition();
	return this->projection * getLookAtMatrix(eye, this->target);
}

Mat4 Camera::updateVP()
{
	this->eye = getCameraPosition();
	if (!this->perspective)
		initOrthographic(this->nearPlane, this->farPlane);
	return this->projection * getLookAtMatrix(eye, this->target);
}

Mat4 Camera::getInverseVP() const
{
	Mat4 invP = getInversePerspective();
	Mat4 invL = getInverseLookAt();
	return invL * invP;
}

Mat4 Camera::getLookAtMatrix(Vec3 eye, Vec3 center) const
{
	float x, y, z;
	float rot = (this->pos.x - this->posDiff.x) * pi / 360.0f;
	Vec3 a = pg::normalize(center - eye);
	Vec3 b(std::sin(rot), 0.0f, -std::cos(rot));
	Vec3 c = pg::cross(b, a);
	x = -pg::dot(b, eye);
	y = -pg::dot(c, eye);
	z = pg::dot(a, eye);
	return Mat4(
		b.x, c.x, -a.x, 0.0f,
		b.y, c.y, -a.y, 0.0f,
		b.z, c.z, -a.z, 0.0f,
		x, y, z, 1.0f);
}

Mat4 Camera::getInverseLookAt() const
{
	Vec3 eye = getCameraPosition();
	Mat4 a = getLookAtMatrix(eye, this->target);
	Mat4 b = pg::identity();

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			b[i][j] = a[j][i];

	Vec3 t = {a[3][0], a[3][1], a[3][2]};
	t = b.apply(t, 0.0f);
	b[3][0] = -t.x;
	b[3][1] = -t.y;
	b[3][2] = -t.z;

	return b;
}

void Camera::initOrthographic(Vec3 near, Vec3 far)
{
	near.x *= this->distance;
	near.y *= this->distance;
	far.x *= this->distance;
	far.y *= this->distance;
	float a = -(far.x+near.x)/(far.x-near.x);
	float b = -(far.y+near.y)/(far.y-near.y);
	float c = -(far.z+near.z)/(far.z-near.z);
	this->projection = Mat4(
		2.0f/(far.x-near.x), 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f/(far.y-near.y), 0.0f, 0.0f,
		0.0f, 0.0f, -2.0f/(far.z-near.z), 0.0f,
		a, b, c, 1.0f);
}

void Camera::setOrthographic(Vec3 near, Vec3 far)
{
	this->perspective = false;
	this->farPlane = far;
	this->nearPlane = near;
	initOrthographic(near, far);
}

Mat4 Camera::getInverseOrthographic() const
{
	float a = this->projection[0][0];
	float b = this->projection[1][1];
	float c = this->projection[2][2];
	float d = this->projection[3][0];
	float e = this->projection[3][1];
	float f = this->projection[3][2];
	return Mat4(
		1.0f/a, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f/b, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f/c, 0.0f,
		-d/a, -e/b, -f/c, 1.0f);
}

void Camera::setPerspective(float fovy, float near, float far, float aspect)
{
	this->perspective = true;
	float t = std::tan((fovy * pi / 180.0f) / 2.0f);
	float a = 1.0f / (aspect * t);
	float b = 1.0f / t;
	float c = -(far + near) / (far - near);
	float d = -1.0f;
	float e = -(2.0f * far * near) / (far - near);
	this->projection = Mat4(
		a, 0.0f, 0.0f, 0.0f,
		0.0f, b, 0.0f, 0.0f,
		0.0f, 0.0f, c, d,
		0.0f, 0.0f, e, 0.0f);
}

Mat4 Camera::getInversePerspective() const
{
	float a = this->projection[0][0];
	float b = this->projection[1][1];
	float c = this->projection[2][2];
	float d = this->projection[2][3];
	float e = this->projection[3][2];
	return Mat4(
		1.0f/a, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f/b, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f/e,
		0.0f, 0.0f, 1.0f/d, -c/(e*d));
}

Vec3 Camera::toScreenSpace(Vec3 point) const
{
	Mat4 vp = getVP();
	Vec4 v = vp * pg::toVec4(point, 1.0f);
	point = pg::toVec3(v);
	point.x /= v.w;
	point.y /= v.w;
	point.x = (point.x + 1.0f) / 2.0f * this->winWidth;
	point.y = this->winHeight - (point.y + 1.0f) / 2.0f * this->winHeight;
	return point;
}

pg::Ray Camera::getRay(int x, int y) const
{
	if (this->perspective)
		return getPerspectiveRay(x, y);
	else
		return getOrthographicRay(x, y);
}

/** The direction of the ray remains constant while the camera remains
stationary. The origin of the ray depends on the screen coordinates. */
pg::Ray Camera::getOrthographicRay(int x, int y) const
{
	Mat4 invProj = getInverseOrthographic();
	Mat4 invLook = getInverseLookAt();

	Vec3 p;
	p.x = 2.0f*x/(this->winWidth-1) - 1.0f;
	p.y = 1.0f - 2.0f*y/(this->winHeight-1);
	p.z = 0.0f;
	p = invProj.apply(p, 1.0f);
	p.z = eye.z;
	p = invLook.apply(p, 1.0f);

	pg::Ray ray;
	ray.direction = getDirection();
	ray.origin = p;
	return ray;
}

/** The direction of the ray depends on the screen coordinates. The origin of
the ray remains stationary at the camera position. */
pg::Ray Camera::getPerspectiveRay(int x, int y) const
{
	Mat4 invProj = getInversePerspective();
	Mat4 invLook = getInverseLookAt();

	Vec3 p;
	p.x = 2.0f*x/(this->winWidth-1) - 1.0f;
	p.y = 1.0f - 2.0f*y/(this->winHeight-1);
	p.z = -1.0f;
	p = invProj.apply(p, 1.0f);
	p.z = -1.0f;
	p = pg::normalize(invLook.apply(p, 0.0f));

	pg::Ray ray;
	ray.direction = p;
	ray.origin = eye;
	return ray;
}

Vec3 Camera::getFar() const
{
	return this->distance * this->farPlane;
}

Vec3 Camera::getNear() const
{
	return this->distance * this->nearPlane;
}

bool Camera::isPerspective() const
{
	return this->perspective;
}

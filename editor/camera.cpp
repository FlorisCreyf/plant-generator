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

#ifdef _WIN32
#undef near
#undef far
#endif

using pg::Vec2;
using pg::Vec3;
using pg::Vec4;
using pg::Mat4;

const float pi = std::acos(-1.0);

Camera::Camera() :
	xAngle(0.0f),
	zAngle(0.0f),
	distance(10.0f),
	panSpeed(0.002f),
	zoomSpeed(0.01f),
	rotateSpeed(pi/360.0f),
	zoomMin(0.1f),
	zoomMax(100.0f),
	target(0.0f, 0.0f, 6.0f),
	action(Action::None),
	perspective(true),
	scaleOrthographic(true)
{

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
	this->xAngle = x;
	this->zAngle = y;
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
		zoom(this->cursor.y - y);
		this->cursor.y = y;
		return true;
	} else if (this->action == Rotate) {
		rotate(x, y);
		return true;
	} else if (this->action == Pan) {
		pan(x, y);
		return true;
	}
	return false;
}

void Camera::setStartCoordinates(float x, float y)
{
	this->cursor = Vec2(x, y);
}

void Camera::rotate(float x, float y)
{
	this->xAngle += (this->cursor.y-y) * this->rotateSpeed;
	this->zAngle += (this->cursor.x-x) * this->rotateSpeed;
	this->cursor = Vec2(x, y);
}

void Camera::pan(float x, float y)
{
	Vec3 eye = getPosition();
	/* The direction from the camera to the target is the normal of the
	plane the camera pans on. */
	Vec3 normal = normalize(eye - target);
	/* The horizontal vector is always on the xy-plane because the camera
	is never tilted. */
	Vec3 horizontal(std::cos(this->zAngle), std::sin(this->zAngle), 0.0f);
	Vec3 vertical = cross(horizontal, normal);
	float speed = this->panSpeed * distance;
	horizontal *= (this->cursor.x - x) * speed;
	vertical *= (this->cursor.y - y) * speed;
	this->target += horizontal + vertical;
	this->cursor = Vec2(x, y);
}

void Camera::setWindowSize(int width, int height)
{
	this->winWidth = width;
	this->winHeight = height;
}

void Camera::scaleOrthographicVolume(bool scale)
{
	this->scaleOrthographic = scale;
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

Vec3 Camera::getPosition() const
{
	float cx = std::cos(-this->xAngle);
	float cz = std::cos(-this->zAngle);
	float sx = std::sin(-this->xAngle);
	float sz = std::sin(-this->zAngle);
	Vec3 eye;
	eye.x = this->distance * sx * sz + this->target.x;
	eye.y = this->distance * sx * cz + this->target.y;
	eye.z = this->distance * cx + this->target.z;
	return eye;
}

Vec3 Camera::getDirection() const
{
	float cx = std::cos(-this->xAngle);
	float cz = std::cos(-this->zAngle);
	float sx = std::sin(-this->xAngle);
	float sz = std::sin(-this->zAngle);
	return Vec3(-sx*sz, -sx*cz, -cx);;
}

Mat4 Camera::getTransform() const
{
	if (this->perspective)
		return getPerspective() * getView();
	else
		return getOrthographic() * getView();
}

Mat4 Camera::getView() const
{
	float cx = std::cos(-this->xAngle);
	float sx = std::sin(-this->xAngle);
	float cz = std::cos(-this->zAngle);
	float sz = std::sin(-this->zAngle);
	Vec3 t = getPosition();
	float tx = -t.x*cz + t.y*sz;
	float ty = -t.x*cx*sz - t.y*cx*cz + t.z*sx;
	float tz = -t.x*sx*sz - t.y*sx*cz - t.z*cx;
	return Mat4(
		cz, cx*sz, sx*sz, 0.0f,
		-sz, cx*cz, sx*cz, 0.0f,
		0.0f, -sx, cx, 0.0f,
		tx, ty, tz, 1.0f);
}

Mat4 Camera::getInverseView() const
{
	float cx = std::cos(this->xAngle);
	float sx = std::sin(this->xAngle);
	float cz = std::cos(this->zAngle);
	float sz = std::sin(this->zAngle);
	Vec3 t = getPosition();
	return Mat4(
		cz, sz, 0.0f, 0.0f,
		-sz*cx, cz*cx, sx, 0.0f,
		sz*sx, -cz*sx, cx, 0.0f,
		t.x, t.y, t.z, 1.0f);
}

void Camera::setOrthographic(Vec3 near, Vec3 far)
{
	this->perspective = false;
	this->near = near;
	this->far = far;
}

Mat4 Camera::getOrthographic() const
{
	Vec3 near = getNear();
	Vec3 far = getFar();
	Vec3 n = near + far;
	Vec3 m = near - far;
	return Mat4(
		2.0f/m.x, 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f/m.y, 0.0f, 0.0f,
		0.0f, 0.0f, 2.0f/m.z, 0.0f,
		-n.x/m.x, -n.y/m.y, -n.z/m.z, 1.0f);
}

Mat4 Camera::getInverseOrthographic() const
{
	Vec3 near = getNear();
	Vec3 far = getFar();
	Vec3 n = near + far;
	Vec3 m = near - far;
	return Mat4(
		m.x/2.0f, 0.0f, 0.0f, 0.0f,
		0.0f, m.y/2.0f, 0.0f, 0.0f,
		0.0f, 0.0f, m.z/2.0f, 0.0f,
		n.x/2.0f, n.y/2.0f, n.z/2.0f, 1.0f);
}

void Camera::setPerspective(float fovy, float near, float far, float aspect)
{
	this->perspective = true;
	this->near = Vec3(aspect, fovy, near);
	this->far = Vec3(aspect, fovy, far);
}

Mat4 Camera::getPerspective() const
{
	float aspect = this->near.x;
	float fovy = this->near.y;
	float near = this->near.z;
	float far = this->far.z;
	float a = (-near-far)/(near-far);
	float b = (2.0f*near*far)/(near-far);
	float c = 1.0f/std::tan(fovy);
	return Mat4(
		c/aspect, 0.0f, 0.0f, 0.0f,
		0.0f, c, 0.0f, 0.0f,
		0.0f, 0.0f, a, -1.0f,
		0.0f, 0.0f, b, 0.0f);
}

Mat4 Camera::getInversePerspective() const
{
	float aspect = this->near.x;
	float fovy = this->near.y;
	float near = this->near.z;
	float far = this->far.z;
	float a = (near-far)/(2.0f*near*far);
	float b = (-near-far)/(2.0f*near*far);
	float c = std::tan(fovy);
	return Mat4(
		c*aspect, 0.0f, 0.0f, 0.0f,
		0.0f, c, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, a,
		0.0f, 0.0f, -1.0f, b);
}

Vec3 Camera::toScreenSpace(Vec3 point) const
{
	Mat4 transform = getTransform();
	Vec4 v = transform * pg::toVec4(point, 1.0f);
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
	Mat4 inverseProjection = getInverseOrthographic();
	Mat4 inverseView = getInverseView();
	Vec3 eye = getPosition();

	Vec3 point;
	point.x = 2.0f*x/(this->winWidth-1) - 1.0f;
	point.y = 1.0f - 2.0f*y/(this->winHeight-1);
	point.z = 0.0f;
	point = inverseProjection.apply(point, 1.0f);
	point.z = eye.z;
	point = inverseView.apply(point, 1.0f);

	pg::Ray ray;
	ray.direction = getDirection();
	ray.origin = point;
	return ray;
}

/** The direction of the ray depends on the screen coordinates. The origin of
the ray remains stationary at the camera position. */
pg::Ray Camera::getPerspectiveRay(int x, int y) const
{
	Mat4 inverseProjection = getInversePerspective();
	Mat4 inverseView = getInverseView();

	Vec3 point;
	point.x = 2.0f*x/(this->winWidth-1) - 1.0f;
	point.y = 1.0f - 2.0f*y/(this->winHeight-1);
	point.z = -1.0f;
	point = inverseProjection.apply(point, 1.0f);
	point.z = -1.0f;
	point = pg::normalize(inverseView.apply(point, 0.0f));

	pg::Ray ray;
	ray.direction = point;
	ray.origin = getPosition();
	return ray;
}

Vec3 Camera::getFar() const
{
	Vec3 far = this->far;
	if (!this->perspective && this->scaleOrthographic) {
		far.x *= this->distance;
		far.y *= this->distance;
	}
	return far;
}

Vec3 Camera::getNear() const
{
	Vec3 near = this->near;
	if (!this->perspective && this->scaleOrthographic) {
		near.x *= this->distance;
		near.y *= this->distance;
	}
	return near;
}

float Camera::getDistance() const
{
	return this->distance;
}

bool Camera::isPerspective() const
{
	return this->perspective;
}

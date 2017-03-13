/* TreeMaker: 3D tree model editor
 * Copyright (C) 2016-2017  Floris Creyf
 *
 * TreeMaker is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TreeMaker is distributed in the hope that it will be useful,
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

using namespace treemaker;

#define UNUSED(x) (void)(x)

Camera::Camera()
{
	action = NONE;
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
	Vec3 a, b;
	Vec3 eye = getCameraPosition();
	Vec3 dir = normalize(eye - target);
	float panSpeed = distance * 0.002f;
	
	a = cross(dir, up);
	b = cross(dir, a);
	
	a = (x - start.x) * panSpeed * a;
	b = (start.y - y) * panSpeed * b;
	
	target = a + b + ftarget;
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

Vec3 Camera::getCameraPosition()
{
	const float toRadian = M_PI / 180.0f;
	float x = (pos.x - posDiff.x + 0.1f) * toRadian * 0.5f;
	float y = (pos.y - posDiff.y + 0.1f) * toRadian * 0.5f;
	eye.x = distance * std::cos(x) * std::cos(y) + target.x;
	eye.y = distance * std::sin(y) + target.y;
	eye.z = distance * std::sin(x) * std::cos(y) + target.z;

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

Vec3 Camera::getPosition()
{
	return eye;
}

Vec3 Camera::getDirection()
{
	return normalize(eye - target);
}

Mat4 Camera::getVP()
{
	Vec3 eye = getCameraPosition();
	Mat4 l = getLookAtMatrix(eye, target, up);
	return perspective * l;
}

Mat4 Camera::getInverseVP()
{
	Mat4 invP = getInversePerspective();
	Mat4 invL = getInverseLookAt();
	return invL * invP;
}

Mat4 Camera::getLookAtMatrix(Vec3 &eye, Vec3 &center, Vec3 &up)
{
	float x, y, z;
	Vec3 a, b, c;

	a = normalize(center - eye);
	b = normalize(cross(a, up));
	c = cross(b, a);

	x = -dot(b, eye);
	y = -dot(c, eye);
	z = dot(a, eye);

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
	Mat4 a = getLookAtMatrix(eye, target, up);
	Mat4 b = identity();

	for (int i = 0; i < 3;i++)
		for (int j = 0; j < 3; j++)
			b[i][j] = a[j][i];

	Vec3 t = {a[3][0], a[3][1], a[3][2]};
	t = toVec3(b * toVec4(t, 0.0f));
	b[3][0] = -t.x;
	b[3][1] = -t.y;
	b[3][2] = -t.z;

	return b;
}

void Camera::setPerspective(float fovy, float near, float far, float aspect)
{
	float t = std::tan((fovy * M_PI / 180.0f) / 2.0f);
	float a = 1.0f / (aspect * t);
	float b = 1.0f / t;
	float c = -(far + near) / (far - near);
	float d = -1.0f;
	float e = -(2.0f * far * near) / (far - near);

	perspective = (Mat4){
		a, 0.0f, 0.0f, 0.0f,
		0.0f, b, 0.0f, 0.0f,
		0.0f, 0.0f, c, d,
		0.0f, 0.0f, e, 0.0f
	};
}

Mat4 Camera::getInversePerspective()
{
	float a = perspective[0][0];
	float b = perspective[1][1];
	float c = perspective[2][2];
	float d = perspective[2][3];
	float e = perspective[3][2];

	return (Mat4){
		1.0f/a, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f/b, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f/e,
		0.0f, 0.0f, 1.0f/d, -c/(e*d)
	};
}

Vec3 Camera::getRayDirection(int x, int y)
{
	Vec3 p;
	Mat4 invP = getInversePerspective();
	Mat4 invL = getInverseLookAt();

	p.x = 2.0f*x/(winWidth-1) - 1.0f;
	p.y = 1.0f - 2.0f*y/(winHeight-1);
	p.z = -1.0f;
	p = toVec3(invP * toVec4(p, 1.0f));
	p.z = -1.0f;
	p = toVec3(invL * toVec4(p, 0.0f));
	p = normalize(p);

	return p;
}

Vec3 Camera::toScreenSpace(Vec3 point)
{
	Mat4 vp = getVP();
	Vec4 v = vp * toVec4(point, 1.0f);
	point = toVec3(v);
	point.x /= v.w;
	point.y /= v.w;
	point.x = (point.x + 1.0f) / 2.0f * winWidth;
	point.y = winHeight - (point.y + 1.0f) / 2.0f * winHeight;
	return point;
}

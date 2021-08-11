/* Copyright 2017 Floris Creyf
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "intersection.h"
#include "mat4.h"
#include "quat.h"
#include <cmath>
#include <limits>

using pg::Aabb;
using pg::Ray;
using pg::Vec3;
using pg::Mat4;
using pg::Obb;
using pg::Plane;

Aabb pg::createAABB(const DVertex *buffer, size_t size)
{
	Aabb aabb(buffer[0].position, buffer[0].position);
	for (size_t i = 1; i < size; i++) {
		Vec3 position = buffer[i].position;
		if (aabb.a.x > position.x)
			aabb.a.x = position.x;
		if (aabb.b.x < position.x)
			aabb.b.x = position.x;
		if (aabb.a.y > position.y)
			aabb.a.y = position.y;
		if (aabb.b.y < position.y)
			aabb.b.y = position.y;
		if (aabb.a.z > position.z)
			aabb.a.z = position.z;
		if (aabb.b.z < position.z)
			aabb.b.z = position.z;
	}
	return aabb;
}

Aabb pg::createAABB(const Vec3 *buffer, size_t size)
{
	Aabb aabb(buffer[0], buffer[0]);
	for (size_t i = 1; i < size; i++) {
		Vec3 position = buffer[i];
		if (aabb.a.x > position.x)
			aabb.a.x = position.x;
		if (aabb.b.x < position.x)
			aabb.b.x = position.x;
		if (aabb.a.y > position.y)
			aabb.a.y = position.y;
		if (aabb.b.y < position.y)
			aabb.b.y = position.y;
		if (aabb.a.z > position.z)
			aabb.a.z = position.z;
		if (aabb.b.z < position.z)
			aabb.b.z = position.z;
	}
	return aabb;
}

Aabb pg::combineAABB(Aabb a, Aabb b)
{
	if (a.a.x > b.a.x)
		a.a.x = b.a.x;
	if (a.b.x < b.b.x)
		a.b.x = b.b.x;
	if (a.a.y > b.a.y)
		a.a.y = b.a.y;
	if (a.b.y < b.b.y)
		a.b.y = b.b.y;
	if (a.a.z > b.a.z)
		a.a.z = b.a.z;
	if (a.b.z < b.b.z)
		a.b.z = b.b.z;
	return a;
}

void swap(float *a, float *b)
{
	float t = *b;
	*b = *a;
	*a = t;
}

float pg::intersectsOBB(Ray &ray, Obb &obb)
{
	float tmin = std::numeric_limits<float>::lowest();
	float tmax = std::numeric_limits<float>::max();
	Vec3 p = obb.center - ray.origin;
	for (int i = 0; i < 3; i++) {
		float e = dot(obb.n[i], p);
		float f = dot(obb.n[i], ray.direction);
		if (std::abs(f) > 0.0001f) {
			float t1 = (e + obb.h[i]) / f;
			float t2 = (e - obb.h[i]) / f;
			if (t1 > t2)
				swap(&t1, &t2);
			if (t1 > tmin)
				tmin = t1;
			if (t2 < tmax)
				tmax = t2;
			if (tmin > tmax)
				return 0.0f;
			if (tmax < 0.0f)
				return 0.0f;
		} else if (-e - obb.h[i] > 0.0f || -e + obb.h[i] < 0.0f)
			return 0.0f;
	}
	return tmin > 0 ? tmin : tmax;
}

float pg::intersectsAABB(Ray &ray, Aabb &aabb)
{
	Obb obb;
	Vec3 a = 0.5f * (aabb.b - aabb.a);
	Vec3 b = 0.5f * (aabb.b + aabb.a);
	obb.h[0] = a.x;
	obb.h[1] = a.y;
	obb.h[2] = a.z;
	obb.center = b;
	obb.n[0] = Vec3(1.0f, 0.0f, 0.0f);
	obb.n[1] = Vec3(0.0f, 1.0f, 0.0f);
	obb.n[2] = Vec3(0.0f, 0.0f, 1.0f);
	return intersectsOBB(ray, obb);
}

float pg::intersectsTriangle(Ray &ray, Vec3 p1, Vec3 p2, Vec3 p3)
{
	float t = 0.0f;
	Vec3 edge1 = p2 - p1;
	Vec3 edge2 = p3 - p1;
	Vec3 q = cross(ray.direction, edge2);
	float a = dot(edge1, q);
	if (a != 0.0f) {
		float f = 1.0f / a;
		Vec3 s = ray.origin - p1;
		float u = f * dot(s, q);
		if (u < 0.0f)
			return 0.0f;
		Vec3 r = cross(s, edge1);
		float v = f * dot(ray.direction, r);
		if (v < 0.0f || u + v > 1.0f)
			return 0.0f;
		t = f * dot(edge2, r);
	}
	return t;
}

float pg::intersectsFrontTriangle(Ray &ray, Vec3 p1, Vec3 p2, Vec3 p3)
{
	float t = 0.0f;
	Vec3 edge1 = p2 - p1;
	Vec3 edge2 = p3 - p1;
	Vec3 q = cross(ray.direction, edge2);
	float a = dot(edge1, q);
	if (a > 0.0f) {
		float f = 1.0f / a;
		Vec3 s = ray.origin - p1;
		float u = f * dot(s, q);
		if (u < 0.0f)
			return 0.0f;
		Vec3 r = cross(s, edge1);
		float v = f * dot(ray.direction, r);
		if (v < 0.0f || u + v > 1.0f)
			return 0.0f;
		t = f * dot(edge2, r);
	}
	return t;
}

float pg::intersectsPlane(Ray &ray, Plane &plane)
{
	float a = dot(plane.normal, ray.direction);
	if (a != 0.0f)
		return dot(plane.point - ray.origin, plane.normal) / a;
	else
		return 0.0f;
}

float pg::intersectsRectangle(Ray ray, Vec3 a, Vec3 b, Vec3 d)
{
	Plane plane;
	plane.normal = normalize(cross(b-a, d-a));
	plane.point = a;
	float t = intersectsPlane(ray, plane);
	if (t == 0.0f)
		return 0.0f;
	Vec3 m = ray.origin + t*ray.direction;
	Vec3 ab = b - a;
	Vec3 ad = d - a;
	Vec3 am = m - a;
	float k = dot(ab, am);
	float l = dot(ad, am);
	if (k > 0 && k < dot(ab, ab) && l > 0 && l < dot(ad, ad))
		return t;
	else
		return 0.0f;
}

float pg::intersectsSphere(Ray &ray, Vec3 position, float radius)
{
	Vec3 l = position - ray.origin;
	float a = dot(l, ray.direction);
	float b = dot(l, l);
	if (a < 0.0f && b > std::pow(radius, 2))
		return 0.0f;
	float c = b - a*a;
	if (c > std::pow(radius, 2))
		return 0.0f;
	float d = std::sqrt(std::pow(radius, 2) - c);
	return b > std::pow(radius, 2) ? a - d : a + d;
}

bool findRoots(float a, float b, float c, float (&roots)[2])
{
	float d = b*b - 4.0*a*c;
	if (d >= 0.0) {
		float s = std::sqrt(d);
		roots[0] = (-b + s) / (2.0*a);
		roots[1] = (-b - s) / (2.0*a);
		return true;
	} else
		return false;
}

float pg::intersectsTaperedCylinder(Ray ray, Vec3 o, Vec3 d, float height,
	float r1, float r2)
{
	Quat q = rotateIntoVecQ(d, Vec3(0.0f, 0.0f, 1.0f));
	d = rotate(q, ray.direction);
	o = rotate(q, ray.origin - o);
	float r = 1.0f / (r1*r1);
	float h = (r1 - r2) / (r1 * height);
	float a = r*(d.x*d.x + d.y*d.y) - h*h*(d.z*d.z);;
	float b = r*(d.x*o.x + d.y*o.y) - h*d.z*(h*o.z - 1.0f);
	float c = r*(o.x*o.x + o.y*o.y) - h*o.z*(h*o.z - 2.0f) - 1.0f;
	float t = 0.0f;
	float roots[2];
	if (findRoots(a, 2.0*b, c, roots)) {
		t = roots[0] < roots[1] ? roots[0] : roots[1];
		float z = t*d.z + o.z;
		if (z < 0 || z > height)
			t = 0.0f;
	} else
		t = 0.0f;

	return t;
}

float pg::intersectsLine(Ray2 a, Ray2 b)
{
	Vec2 p = perp(b.direction);
	float d = dot(a.direction, p);
	float t = d != 0 ? dot(b.origin - a.origin, p) / d : -1.0f;
	return t;
}

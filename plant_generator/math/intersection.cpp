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
#include <cmath>
#include <stdlib.h>
#include <float.h>

using pg::Aabb;
using pg::Ray;
using pg::Vec3;
using pg::Mat4;
using pg::Obb;
using pg::Plane;

Aabb pg::createAABB(const pg::Vertex *buffer, size_t size)
{
	Aabb aabb = {buffer[0].position, buffer[0].position};

	for (size_t i = 0; i < size; i += 6) {
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

void swap(float *a, float *b)
{
	float t = *b;
	*b = *a;
	*a = t;
}

float pg::intersectsOBB(Ray &ray, Obb &obb)
{
	float tmin = FLT_MIN;
	float tmax = FLT_MAX;
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
	obb.n[0] = {1.0f, 0.0f, 0.0f};
	obb.n[1] = {0.0f, 1.0f, 0.0f};
	obb.n[2] = {0.0f, 0.0f, 1.0f};
	return pg::intersectsOBB(ray, obb);
}

float pg::intersectsTriangle(Ray &ray, Vec3 p1, Vec3 p2, Vec3 p3)
{
	float t = 0.0f;
	Vec3 edge1 = p2 - p1;
	Vec3 edge2 = p3 - p1;
	Vec3 q = cross(ray.direction, edge2);
	float a = dot(edge1, q);

	if (std::abs(a) > 0.0001f) {
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

/** The following equations are used to compute 't':
... ray(t) = origin + t*direction
... (ray(t) - rectangle_point) `dot` rectangle_normal = 0 */
float pg::intersectsPlane(Ray &ray, Plane &plane)
{
	float a = dot(plane.normal, ray.direction);
	if (a > 0.0f)
		return dot(plane.point - ray.origin, plane.normal) / a;
	else
		return 0.0f;
}

/** Intersect a ray with a plane and determine if the intersected point is
within two edges of the rectangle on that plane. */
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

	if (a < 0.0f && b > pow(radius, 2))
		return 0.0f;

	float c = b - a*a;
	if (c > pow(radius, 2))
		return 0.0f;

	float d = sqrt(pow(radius, 2) - c);
	return b > pow(radius, 2) ? a - d : a + d;
}

bool findRoots(float a, float b, float c, float (&roots)[2])
{
	float discr = b*b - 4.0*a*c;
	if (discr >= 0.0) {
		float sq = sqrt(discr);
		roots[0] = (-b + sq) / (2.0*a);
		roots[1] = (-b - sq) / (2.0*a);
		return true;
	} else
		return false;
}

float pg::intersectsTaperedCylinder(
	Ray ray, Vec3 start, Vec3 direction, float height, float r1, float r2)
{
	Vec3 s = {1.0f/r1, (1.0f - r2/r1)/height, 1.0f/r1};
	Vec3 p = {0.0f, -1.0f, 0.0f};

	float a = s.x*s.x;
	float b = -s.y*s.y;
	float c = s.z*s.z;
	float g = 2.0f*s.x*p.x;
	float h = -2.0f*s.y*p.y;
	float i = 2.0f*s.z*p.z;
	float j = p.x*p.x - p.y*p.y + p.z*p.z;

	{ /* Transform ray into object space of cone. */
		Vec3 yaxis = {0.0f, 1.0f, 0.0f};
		Mat4 m = rotateIntoVec(direction, yaxis);
		ray.origin = ray.origin - start;
		ray.direction = toVec3(m * toVec4(ray.direction, 0.0f));
		ray.origin = toVec3(m * toVec4(ray.origin, 1.0f));
	}

	float aq = 0.0f, bq = 0.0f, cq = 0.0f;
	aq += a*ray.direction.x*ray.direction.x;
	aq += b*ray.direction.y*ray.direction.y;
	aq += c*ray.direction.z*ray.direction.z;
	bq += 2.0f*a*ray.direction.x*ray.origin.x + g*ray.direction.x;
	bq += 2.0f*b*ray.direction.y*ray.origin.y + h*ray.direction.y;
	bq += 2.0f*c*ray.direction.z*ray.origin.z + i*ray.direction.z;
	cq += a*ray.origin.x*ray.origin.x + g*ray.origin.x;
	cq += b*ray.origin.y*ray.origin.y + h*ray.origin.y;
	cq += c*ray.origin.z*ray.origin.z + i*ray.origin.z;
	cq += j;

	float t = 0.0f;
	float roots[2];
	if (findRoots(aq, bq, cq, roots)) {
		t = roots[0] < roots[1] ? roots[0] : roots[1];

		/* The intersection fails if the intersection falls under the
		y-axis or above the height value. The cone is always along
		the y-axis because the ray is transformed into the object
		space of the cone. */
		float y = (t*ray.direction.y + ray.origin.y);
		if (y < 0 || y > height)
			t = 0.0f;
	} else
		t = 0.0f;

	return t;
}

float pg::intersectsLine(pg::Ray2 a, pg::Ray2 b)
{
	Vec2 p = perp(b.direction);
	float d = dot(a.direction, p);
	float t = d != 0 ? dot(b.origin - a.origin, p) / d : -1.0f;
	return t;
}

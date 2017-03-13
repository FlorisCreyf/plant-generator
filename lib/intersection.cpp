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

using namespace treemaker;

Aabb treemaker::createAABB(const float *buffer, size_t size)
{
	Aabb aabb = {
		{buffer[0], buffer[1], buffer[2]},
		{buffer[0], buffer[1], buffer[2]}
	};

	for (size_t i = 0; i < size; i += 6) {
		if (aabb.a.x > buffer[i])
			aabb.a.x = buffer[i];
		if (aabb.b.x < buffer[i])
			aabb.b.x = buffer[i];
		if (aabb.a.y > buffer[i+1])
			aabb.a.y = buffer[i+1];
		if (aabb.b.y < buffer[i+1])
			aabb.b.y = buffer[i+1];
		if (aabb.a.z > buffer[i+2])
			aabb.a.z = buffer[i+2];
		if (aabb.b.z < buffer[i+2])
			aabb.b.z = buffer[i+2];
	}

	return aabb;
}

void swap(float *a, float *b)
{
        float t = *b;
        *b = *a;
        *a = t;
}

float treemaker::intersectsOBB(Ray &ray, Obb &obb)
{
        float e, f, t1, t2;
        float tmin = FLT_MIN;
        float tmax = FLT_MAX;
        Vec3 p = obb.center - ray.origin;

        for (int i = 0; i < 3; i++) {
                e = dot(obb.n[i], p);
                f = dot(obb.n[i], ray.direction);

                if (std::abs(f) > 0.0001f) {
                        t1 = (e + obb.h[i]) / f;
                        t2 = (e - obb.h[i]) / f;

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

float treemaker::intersectsAABB(Ray &ray, Aabb &aabb)
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

        return treemaker::intersectsOBB(ray, obb);
}

float treemaker::intersectsPlane(Ray &ray, Plane &plane)
{
        float a = dot(plane.normal, ray.direction);
        if (a > 0.0f)
                return dot(plane.point - ray.origin, plane.normal) / a;
        return 0.0f;
}

float treemaker::intersectsSphere(Ray &ray, Vec3 position, float radius)
{
        Vec3 l = position - ray.origin;
        float a = dot(l, ray.direction);
        float b = dot(l, l);
        float c;
        float d;

        if (a < 0.0f && b > pow(radius, 2))
                return 0.0f;
        c = b - a*a;
        if (c > pow(radius, 2))
                return 0.0f;
        d = sqrt(pow(radius, 2) - c);

        return b > pow(radius, 2) ? a - d : a + d;
}

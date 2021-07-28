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

#ifndef PG_INTERSECTION_H
#define PG_INTERSECTION_H

#include "vec3.h"
#include "../vertex.h"
#include <cstddef>

namespace pg {
	struct Ray2 {
		Vec2 origin;
		Vec2 direction;
		Ray2() {}
		Ray2(Vec2 o, Vec2 d) : origin(o), direction(d) {}
	};

	struct Ray {
		Vec3 origin;
		Vec3 direction;
		Ray() {}
		Ray(Vec3 o, Vec3 d) : origin(o), direction(d) {}
	};

	/* Axis aligned bounding box (AABB) */
	struct Aabb {
		Vec3 a;
		Vec3 b;
		Aabb() {}
		Aabb(Vec3 a, Vec3 b) : a(a), b(b) {}
	};

	/* Oriented bounding box (OBB) */
	struct Obb {
		float h[3];
		Vec3 n[3];
		Vec3 center;
	};

	struct Plane {
		Vec3 point;
		Vec3 normal;
	};

	Aabb createAABB(const DVertex *buffer, size_t size);
	Aabb createAABB(const Vec3 *buffer, size_t size);
	Aabb combineAABB(Aabb a, Aabb b);
	float intersectsOBB(Ray &ray, Obb &obb);
	float intersectsAABB(Ray &ray, Aabb &aabb);
	float intersectsTriangle(Ray &ray, Vec3 p1, Vec3 p2, Vec3 p3);
	float intersectsFrontTriangle(Ray &ray, Vec3 p1, Vec3 p2, Vec3 p3);
	float intersectsPlane(Ray &ray, Plane &plane);
	float intersectsRectangle(Ray ray, Vec3 a, Vec3 b, Vec3 d);
	float intersectsSphere(Ray &ray, Vec3 position, float radius);
	float intersectsTaperedCylinder(Ray ray, Vec3 start, Vec3 direction,
		float height, float r1, float r2);
	float intersectsLine(Ray2 a, Ray2 b);
}

#endif

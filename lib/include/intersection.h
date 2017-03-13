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

#ifndef TM_INTERSECTION_H
#define TM_INTERSECTION_H

#include "math.h"
#include <cstddef>

namespace treemaker {

	struct Ray {
		Vec3 origin;
		Vec3 direction;
	};

	struct Aabb {
		Vec3 a;
		Vec3 b;
	};

	struct Obb {
		float h[3];
		Vec3 n[3];
		Vec3 center;
	};

	struct Plane {
		Vec3 point;
		Vec3 normal;
	};

	Aabb createAABB(const float *buffer, size_t size);
	float intersectsOBB(Ray &ray, Obb &obb);
	float intersectsAABB(Ray &ray, Aabb &aabb);
	float intersectsPlane(Ray &ray, Plane &plane);
	float intersectsSphere(Ray &ray, Vec3 position, float radius);
}

#endif /* TM_INTERSECTION_H */

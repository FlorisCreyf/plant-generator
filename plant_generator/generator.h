/* Copyright 2020 Floris Creyf
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

#ifndef PG_GENERATOR_H
#define PG_GENERATOR_H

#include "plant.h"
#include "mesh.h"
#include "math/intersection.h"
#include <vector>
#include <map>

namespace pg {
	class Generator {
		struct Light {
			Vec3 direction;
			int rays;
		};
		struct Intersection {
			Stem *stem;
			float t;
		};

		Plant *plant;
		std::map<Stem *, Light> growth;
		float primaryGrowthRate;
		float secondaryGrowthRate;
		float minRadius;
		int rayCount;
		int rayLevels;
		float width;
		Vec2 maxSwelling;

		int propagate(Stem *stem);
		void addStems(Stem *stem);
		void addNodes(int);
		void addNode(Stem *, Light, int);
		void addLeaves(Stem *, float);
		Leaf createLeaf();

		Intersection intersect(Stem *, Ray);
		void castRays();
		void updateGrowth(Ray);
		void updateBoundingBox(Vec3 point);

	public:
		Generator(Plant *plant);
		void grow(int cycles, int nodes);
		void setPrimaryGrowthRate(float rate);
		void setSecondaryGrowthRate(float rate);
		void setRayDensity(int baseCount, int levels);
	};
}

#endif

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
#include "mesh/mesh.h"
#include "volume.h"
#include "math/intersection.h"
#include <vector>
#include <map>
#include <random>

namespace pg {
	class Generator {
		Plant *plant;
		float width;
		Volume volume;
		std::mt19937 mt;

		Stem *createRoot();
		void addToVolume(Volume *, Stem *);
		void castRays(Volume *);
		void updateRadiantEnergy(Volume *, Ray);
		float setConcentration(Stem *);
		void generalizeDensity(Volume::Node *);
		void generalizeFlux(Volume::Node *);
		float evaluateEfficiency(Volume *, Stem *);
		void addNodes(Volume *, Stem *);
		void addNode(Volume *, Stem *);
		void updateRadius(Stem *, float);
		void addStems(Stem *, Volume *);
		void addStem(Stem *, Volume *);
		void addLeaves(Stem *, int);
		Leaf createLeaf();
		void updateBoundingBox(Vec3);

	public:
		float primaryGrowthRate;
		float secondaryGrowthRate;
		float minRadius;
		float suppression;
		float suppressionFalloff;
		float synthesisRate;
		float synthesisThreshold;
		float optimization;
		int depth;
		int rays;
		int cycles;
		int nodes;
		int seed;

		Generator(Plant *plant);
		void grow();
		void clearVolume();
		const Volume *getVolume();
	};
}

#endif

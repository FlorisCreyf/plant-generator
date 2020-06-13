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

#ifndef PG_PSEUDO_GENERATOR_H
#define PG_PSEUDO_GENERATOR_H

#include "plant.h"
#include <random>

namespace pg {
	class PseudoGenerator {
		Plant *plant;
		std::mt19937 randomGenerator;
		int stemDepth;
		float stemDensity;
		float leafDensity;
		float stemStart;
		float leafStart;
		float requiredLength;
		unsigned seed;

		Vec3 getStemDirection(Stem *);
		bool setPath(Stem *, Stem *, Vec3, float);
		void addLateralStems(Stem *);
		bool growLateralStem(Stem *, float);
		void alternateLeaf(Leaf *, Quat);

	public:
		PseudoGenerator(Plant *plant);
		void grow();
		void grow(Stem *stem);
		void setSeed(unsigned seed);
		void reset();
		void setRequiredLength(float length);
		void setStemCount(float density, float start);
		void setLeafCount(float density, float start);
		void setDepth(int depth);
	};
}

#endif /* PG_PSEUDO_GENERATOR_H */

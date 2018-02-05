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

#ifndef PG_GENERATOR_H
#define PG_GENERATOR_H

#include "plant.h"
#include <random>

namespace pg {
	class Generator {
		Plant *plant;
		std::mt19937 randomGenerator;
		int maxStemDepth;

		Vec3 getStemDirection(Stem *stem);
		void getDichotomousDirections(Stem *parent, Vec3 directions[2]);
		void setPath(Stem *stem, Vec3 direction);
		float getRadius(Stem *stem);
		void addLateralStems(Stem *parent, float position);
		void addDichotomousStems(Stem *stem);

	public:
		Generator(Plant *plant);
		void growLateralStem(Stem *stem, float position);
		void grow();

		void setMaxDepth(int depth);
		int getMaxDepth();
	};
}

#endif /* PG_GENERATOR_H */

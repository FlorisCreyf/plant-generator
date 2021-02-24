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

#ifndef PG_PATTERN_GENERATOR_H
#define PG_PATTERN_GENERATOR_H

#include "plant.h"
#include <random>

namespace pg {
	class PatternGenerator {
		Plant *plant;
		ParameterTree parameterTree;
		std::mt19937 mt;

		Vec3 getForkDirection(Stem *, const StemData &, int);
		Vec3 getStemDirection(Stem *, const StemData &, int);
		void addLateralStems(Stem *, float, const ParameterNode *);
		void addLateralStem(Stem *, float, const ParameterNode *, int);
		float addStems(Stem *, Vec3, float, const ParameterNode *);
		float setPath(Stem *, Vec3, float, const StemData &);
		void addLeaves(Stem *, LeafData, float);

	public:
		PatternGenerator(Plant *plant);
		void grow();
		void grow(Stem *stem);
		void reset();
		void setParameterTree(ParameterTree parameterTree);
		ParameterTree getParameterTree() const;
	};
}

#endif

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
		struct Length {
			float current;
			float total;
			Length(float c, float t) : current(c), total(t) {}
		};

		Plant *plant;
		ParameterTree parameterTree;
		std::mt19937 mt;

		void addLateralStems(Stem *, Length, const ParameterNode *);
		void addLateralStem(Stem *, float, Length, int, Vec3 &, Vec3 &,
			const ParameterNode *);
		float modifyRadius(const StemData &, float);
		Vec3 getDirection(Stem *, int, Length, Vec3, Vec3,
			const StemData &);
		Vec3 getForkDirection(Stem *, float, const StemData &);
		float addStems(Stem *, float, float, const ParameterNode *);
		float getCollarLength(Stem *, Vec3);
		float setPath(Stem *, Vec3, float, const StemData &);
		float bifurcatePath(Stem *, int, int, const StemData &);
		void addLeaves(Stem *, Length, LeafData);
		bool occurs(float);

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

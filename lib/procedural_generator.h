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

#ifndef TM_PROC_GENERATOR_H
#define TM_PROC_GENERATOR_H

#include "stem.h"

namespace treemaker {
	class TreeImpl;
}

using namespace treemaker;

class ProcGenerator {
	TreeImpl *tree;
	Vec3 getStemDirection(Stem *stem);
	void getDichotomousDirections(Stem *parent, Vec3 directions[2]);
	void setPath(Stem *stem, Vec3 direction);
	void setRadiusCurve(Stem *stem);
	void addLateralStems(Stem *parent, float position);
	void addDichotomousStems(Stem *stem);

public:
	ProcGenerator(TreeImpl *tree);
	void updateStemDensity(Stem *stem);
	void updateBaseLength(Stem *stem);
	void generateTree();
};

#endif /* TM_PROC_GENERATOR_H */

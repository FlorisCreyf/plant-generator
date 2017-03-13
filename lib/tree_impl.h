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

#ifndef TM_TREE_H
#define TM_TREE_H

#include "stem.h"
#include "procedural_generator.h"
#include "mesh_generator.h"
#include <vector>

namespace treemaker {

	class TreeImpl {
		Stem root;
                unsigned name = 1;

		Stem *findStemByIndex(Stem *parent, size_t &index);
		Stem *findStem(Stem *parent, unsigned name);

	public:
		unsigned maxStemDepth = 2;
		float crownBaseHeight = 1.0f;
		MeshGenerator meshGenerator;
		ProcGenerator procGenerator;
		
		TreeImpl();
		Stem *getRoot();
		Stem *findStemByIndex(size_t index);
		Stem *findStem(unsigned name);

		Stem *addStem(Stem *parent);
                void addDichotomousStems(Stem *parent);
                void removeDichotomousStems(Stem *parent);
		Stem *moveStem(Stem *stem, Stem *parent);
		void removeStem(Stem *stem);
		void removeLateralStems(Stem *parent);
	};
}

#endif /* TM_TREE_H */

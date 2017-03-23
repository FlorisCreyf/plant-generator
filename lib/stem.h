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

#ifndef TM_STEM_H
#define TM_STEM_H

#include "math.h"
#include "curve.h"
#include "path.h"
#include "name_generator.h"
#include "types.h"
#include <vector>
#include <random>

namespace treemaker {
	class TreeImpl;
}

using namespace treemaker;

class Stem {
	unsigned name;
	Stem *parent;
	std::vector<Stem> children;
	bool hasDichotomous = false;
	bool procedural = true;
	int depth;
	int resolution = 10;
	float stemDensity = 0.0f;
	float position;
	Vec3 location;
	Path path;

	friend class treemaker::TreeImpl;

	Stem(NameGenerator &nameGenerator, Stem *parent);
	void modifyResolutions(Stem *stem, int resolution);
	void updatePositions(Stem *stem);

public:	
	std::mt19937 generator;
	Flags mode = AUTOMATIC;
	Flags distribution = DISTRIBUTED;
	
	std::vector<Vec3> radiusCurve;
	float radius = 0.4f;
	float minRadius = 0.01f;
	float baseLength = 0.0f;

	size_t vertexStart;
	size_t vertexCount;
	size_t indexStart;
	size_t indexCount;

	unsigned getName();
	
	void setPath(Path &path);
	Path getPath();
	
	/** The position can only be set if the stem is lateral. */
	void setPosition(float position);
	float getPosition();
	Vec3 getLocation();
	
	void setResolution(int resolution);
	int getResolution();
	
	void setStemDensity(float density, NameGenerator &ng);
	float getStemDensity();
	
	Stem *addLateralStem(NameGenerator &ng);
	void removeLateralStem(size_t index);
	void removeLateralStems();
	bool isLateral();
	
	void addDichotomousStems(NameGenerator &ng);
	void removeDichotomousStems();
	Stem *getDichotomousStem(size_t index);
	bool hasDichotomousStems();
	
	Stem *getParent();
	Stem *getChild(size_t index);
	/** Excludes dichotomous stems */
	size_t getChildCount();
	
	bool isDescendantOf(Stem *stem);
	int getDepth();
};

#endif /* TM_STEM_H */

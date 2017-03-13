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
#include <vector>

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
	float position;
	Vec3 location;
	Path path;

	friend class treemaker::TreeImpl;

	Stem(unsigned name, Stem *parent);
	void modifyResolutions(Stem *stem, int resolution);
	void updatePositions(Stem *stem);

public:
	std::vector<Vec3> radiusCurve;
	float radius = 0.4f;
	float minRadius = 0.01f;
	float stemDensity = 0.0f;

	size_t vertexStart;
	size_t vertexCount;
	size_t indexStart;
	size_t indexCount;

	unsigned getName();

	void setPath(Path &path);
	/** The position can only be set if the stem is lateral. */
	void setPosition(float position);
	/** The location can only be set if the stem isn't lateral. */
	void setLocation(Vec3 location);
	void setResolution(int resolution);

	bool isDescendantOf(Stem *stem);
	bool isLateral();
	Stem *getParent();
	Stem *getChild(size_t index);
	/** Excludes dichotomous stems */
	size_t getChildCount();
	Stem *getDichotomousStem(size_t index);
	bool hasDichotomousStems();
	Path getPath();
	float getPosition();
	Vec3 getLocation();
	int getResolution();
	int getDepth();
};

#endif /* TM_STEM_H */

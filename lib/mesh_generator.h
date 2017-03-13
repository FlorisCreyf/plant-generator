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

#ifndef TM_MESH_GENERATOR_H
#define TM_MESH_GENERATOR_H

#include "stem.h"
#include <vector>

using namespace treemaker;

class MeshGenerator {
	bool resized = true;
	std::vector<float> vertices;
	std::vector<unsigned> indices;
	size_t vbIndex;
	size_t ibIndex;

	void resizeVertexBuffer(size_t additions);
	void resizeIndexBuffer(size_t additions);

	void addCrossSectionPoints(Mat4 tran, float radius, int div);
	float getRadius(Stem *stem, size_t index);
	Mat4 getCrossSectionTransform(Stem *stem, size_t i, float offset);
	void addCrossSection(Stem *stem, size_t i, float offset = 0.0f);
	/* Create a rectangle between cross sections a and b. */
	void addRectangle(size_t *csA, size_t *csB);
	void addLastRectangle(size_t a, size_t b, size_t iA, size_t iB);
	void addTriangleRing(size_t csA, size_t csB, int divisions);
	void addRectangles(size_t csA, size_t csB, int start, int end,
			int divisions);
	void capStem(Stem *stem, size_t crossSection);
	Vec3 getCrossSectionNormal(size_t index);
	/** Angle of stem on cross section plane. */
	float getStemAngle(size_t index, Vec3 direction);
	/** 
	 * Determine what vertices connect the stem to the parent. Lower and
	 * upper bounds need to be signed because they might become negative 
	 * during computations. 
	 */
	void getBounds(Vec3 direction, int divisions, size_t index,
			int *lowerBound, int *upperBound, float *angle);
	/** This limits the max number of vertices per cross section to 180. */
	bool isDichotomousTwisted(float angle, int divisions);
	/** Connects three cross sections labeled a, b, and c. */
	void connectDichotomous(size_t a, size_t b, size_t c,
			int divisions, Vec3 direction);
	void addDichotomousStems(Stem *parent, size_t crossSection);
	/** Stems at the end of the parent stem need a small offset. */
	void addStem(Stem *stem, float offset = 0.0f);

public:
	MeshGenerator();
	bool generate(Stem *stem);
	const float *getVertices();
	size_t getVertexCount();
	size_t getVertexCapacity();
	const unsigned *getIndices();
	size_t getIndexCount();
	size_t getIndexCapacity();
};

#endif /* TM_MESH_GENERATOR_H */

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

#ifndef PG_MESH_H
#define PG_MESH_H

#include "stem.h"
#include "plant.h"
#include <vector>

namespace pg {
	struct Segment {
		Stem *stem;
		size_t vertexStart;
		size_t indexStart;
		size_t vertexCount;
		size_t indexCount;
	};

	class Mesh {
		Plant *plant;
		const int vertexSize = 6;
		std::vector<float> vertices;
		std::vector<unsigned> indices;
		std::vector<Segment> segments;

		void addSectionPoints(Mat4 t, float radius, int count);
		Mat4 getSectionTransform(Stem *stem, size_t section,
			float offset);
		void addSection(Stem *stem, size_t section,
			float offset = 0.0f);
		/* Create a rectangle between cross sections a and b. */
		void addRectangle(size_t *s1, size_t *s2);
		void addLastRectangle(size_t a, size_t b, size_t ia, size_t ib);
		void addTriangleRing(size_t s1, size_t s2, int divisions);
		void addRectangles(size_t s1, size_t s2, int start, int end,
			int divisions);
		Vec3 getCrossSectionNormal(size_t index);
		/** Angle of stem on cross section plane. */
		float getStemAngle(size_t index, Vec3 direction);
		/** Determine what vertices connect the stem to the parent. */
		void getBounds(Vec3 direction, int divisions, size_t index,
			int *lowerBound, int *upperBound, float *angle);
		/** This limits the number of vertices per section to 180. */
		bool isDichotomousTwisted(float angle, int divisions);
		/** Connects three cross sections labeled a, b, and c. */
		void connectDichotomous(size_t a, size_t b, size_t c,
			int divisions, Vec3 direction);
		void addDichotomousStems(Stem *parent, size_t section);
		void capStem(Stem *stem, size_t section);
		/** Stems at the end of the parent stem need a small offset. */
		void addStem(Stem *stem, float offset = 0.0f);

	public:
		Mesh(Plant *plant);
		bool generate();
		const std::vector<float> *getVertices() const;
		const std::vector<unsigned> *getIndices() const;
		const std::vector<Segment> *getSegments() const;
		Segment find(Stem *stem) const;
	};
}

#endif /* PG_MESH_H */

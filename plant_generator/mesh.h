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
#include <map>

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
		const int vertexSize = 8;

		int mesh;
		std::map<unsigned, unsigned> meshes;
		std::vector<unsigned> materials;
		std::vector<std::vector<float>> vertices;
		std::vector<std::vector<unsigned>> indices;
		std::vector<std::vector<Segment>> segments;

		Mat4 getSectionTransform(Stem *stem, size_t section,
			float offset);
		void addSection(Stem *stem, size_t section,
			float &textureOffset, float pathOffset = 0.0f);
		/* Create a rectangle between cross sections a and b. */
		void addRectangle(size_t *s1, size_t *s2);
		void addLastRectangle(size_t a, size_t b, size_t ia, size_t ib);
		void addTriangleRing(size_t s1, size_t s2, int divisions);
		void addRectangles(size_t s1, size_t s2, int start, int end,
			int divisions);
		void capStem(Stem *stem, int stemMesh, size_t section);
		/** Stems at the end of the parent stem need a small offset. */
		void addStem(Stem *stem, int mesh, float offset = 0.0f);
		void addLeaves(Stem *stem);
		void rotateSideLeaf(Vec3 (&p)[4], Vec3 &normal, Vec3 direction);
		void rotateEndLeaf(Vec3 (&p)[4], Vec3 &normal, Vec3 direction);
		void addPoint(Vec3 point, Vec3 normal, Vec2 texture);
		void addTriangle(int a, int b, int c);
		int selectBuffer(int material, int mesh);

	public:
		Mesh(Plant *plant);
		void generate();
		std::vector<float> getVertices() const;
		std::vector<unsigned> getIndices() const;
		const std::vector<float> *getVertices(int mesh) const;
		const std::vector<unsigned> *getIndices(int mesh) const;
		const std::vector<Segment> *getSegments(int mesh) const;
		Segment find(Stem *stem) const;
		int getVertexCount() const;
		int getIndexCount() const;
		int getMeshCount() const;
		unsigned getMaterialId(int mesh);
	};
}

#endif /* PG_MESH_H */

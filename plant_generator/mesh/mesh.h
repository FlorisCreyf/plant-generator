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

#include "../cross_section.h"
#include "../math/intersection.h"
#include "../plant.h"
#include "../stem.h"
#include "../vertex.h"
#include <map>
#include <utility>
#include <vector>

namespace pg {
	class Mesh {
	public:
		struct Segment {
			Stem *stem;
			size_t leafIndex;
			size_t vertexStart;
			size_t indexStart;
			size_t vertexCount;
			size_t indexCount;
		};
		struct State {
			Segment segment;
			Vec3 prevDirection;
			Quat prevRotation;
			size_t prevIndex;
			size_t section;
			size_t jointIndex;
			int jointID;
			int mesh;
			float texOffset;
			float jointOffset;
		};

		using LeafID = std::pair<Stem *, size_t>;

		Mesh(Plant *plant);
		Mesh(const Mesh &original) = delete;
		Mesh &operator=(const Mesh &original) = delete;

		std::vector<DVertex> getVertices() const;
		std::vector<unsigned> getIndices() const;
		const std::vector<DVertex> *getVertices(int mesh) const;
		const std::vector<unsigned> *getIndices(int mesh) const;
		/** Find the location of a stem in the buffer. */
		Segment findStem(Stem *stem) const;
		/** Find the location of a leaf in the buffer. */
		Segment findLeaf(LeafID leaf) const;
		std::map<LeafID, Segment> getLeaves(int mesh) const;
		size_t getLeafCount(int mesh) const;
		size_t getVertexCount() const;
		size_t getIndexCount() const;
		size_t getMeshCount() const;
		unsigned getMaterialIndex(int mesh) const;

	private:
		Plant *plant;
		CrossSection section;
		std::vector<std::vector<DVertex>> vertices;
		std::vector<std::vector<unsigned>> indices;
		std::vector<std::map<Stem *, Segment>> stems;
		std::vector<std::map<LeafID, Segment>> leaves;

		size_t insertTriangleRing(size_t, size_t, int, unsigned *);
		void addTriangleRing(size_t, size_t, int, int);
		void addTriangle(int, int, int, int);
		void initBuffer();
		void updateSegments();

		friend class MeshGenerator;
		friend class Collar;
		friend class Fork;
	};
}

#endif

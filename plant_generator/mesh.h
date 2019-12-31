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
#include "vertex.h"
#include <vector>
#include <map>

namespace pg {
	struct Segment {
		Stem *stem;
		long leaf;
	 	unsigned vertexStart;
	 	unsigned indexStart;
	 	unsigned vertexCount;
	 	unsigned indexCount;
	};

	class Mesh {
		Plant *plant;
		Geometry defaultLeaf;

		int mesh;
		std::map<long, int> meshes;
		std::vector<long> materials;
		std::vector<std::vector<Vertex>> vertices;
		std::vector<std::vector<unsigned>> indices;
		std::vector<std::map<long, Segment>> stemSegments;
		std::vector<std::map<long, Segment>> leafSegments;

		Mat4 getSectionTransform(Stem *, size_t);
		void addSection(Stem *, size_t, float &);
		void addTriangleRing(size_t, size_t, int);
		void capStem(Stem *, int, size_t);
		Segment addStem(Stem *);
		void addLeaves(Stem *);
		void addLeaf(Leaf *leaf, Stem *stem);
		void addTriangle(int, int, int);
		int selectBuffer(int);
		void initBuffer();
		void updateSegments();

	public:
		Mesh(Plant *plant);
		void generate();
		std::vector<Vertex> getVertices() const;
		std::vector<unsigned> getIndices() const;
		const std::vector<Vertex> *getVertices(int mesh) const;
		const std::vector<unsigned> *getIndices(int mesh) const;
		Segment findStem(Stem *stem) const;
		Segment findLeaf(long leaf) const;
		std::map<long, Segment> getLeaves(int mesh) const;
		int getLeafCount(int mesh) const;
		int getVertexCount() const;
		int getIndexCount() const;
		int getMeshCount() const;
		long getMaterialID(int mesh) const;
	};
}

#endif /* PG_MESH_H */

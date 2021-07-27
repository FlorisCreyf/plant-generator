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

#include "cross_section.h"
#include "stem.h"
#include "plant.h"
#include "math/intersection.h"
#include "vertex.h"
#include <vector>
#include <map>
#include <utility>

namespace pg {
	struct Segment {
		Stem *stem;
		size_t leafIndex;
		size_t vertexStart;
		size_t indexStart;
		size_t vertexCount;
		size_t indexCount;
	};

	class Mesh {
	public:
		using LeafID = std::pair<Stem *, size_t>;

		Mesh(Plant *plant);
		Mesh(const Mesh &original) = delete;
		Mesh &operator=(const Mesh &original) = delete;

		void generate();
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

		Plant *plant;
		CrossSection crossSection;

		std::vector<std::vector<DVertex>> vertices;
		std::vector<std::vector<unsigned>> indices;
		std::vector<std::map<Stem *, Segment>> stemSegments;
		std::vector<std::map<LeafID, Segment>> leafSegments;

		void addSections(State &, Segment, bool, Stem *);
		void addSection(State &, Quat, const CrossSection &);
		float getTextureLength(Stem *, size_t);
		float getTextureLength(Stem *, size_t, size_t);
		void setInitialRotation(Stem *, State &);
		Quat rotateSection(State &);
		void capStem(Stem *, int, size_t);
		Segment addStem(Stem *, State &, State, bool);
		void addChildStems(Stem *, Stem *[2], State &);

		void addForks(Stem *[2], State &);
		void createFork(Stem *, State &);
		void reserveForkSpace(const Stem *, int);
		int getForkMidpoint(int, Vec3, Vec3, Vec3, Quat);
		void addForkTriangles(const State &, const State [2],
			Stem *[2], const Segment [2]);

		void createBranchCollar(State &, Segment);
		size_t insertCollar(Segment, Segment, size_t);
		void reserveBranchCollarSpace(Stem *, int);
		Mat4 getBranchCollarScale(Stem *, Stem *);
		DVertex moveToSurface(DVertex, Ray, Segment, size_t);
		DVertex moveToForkSurface(DVertex, Ray, Segment);
		void setBranchCollarNormals(size_t, size_t, int, int, int);
		void setBranchCollarUVs(size_t, Stem *, int, int, int);
		void connectCollar(const State &, bool);

		void addLeaves(Stem *, const State &);
		void addLeaf(Stem *, unsigned, const State &);
		Geometry transformLeaf(const Leaf *, const Stem *);

		void setInitialJointState(State &, const State &);
		std::pair<size_t, Joint> getJoint(float, const Stem *);
		void incrementJoint(State &, const std::vector<Joint> &);
		void updateJointState(State &, Vec2 &, Vec2 &);
		void setJointInfo(const Stem *, float, size_t, Vec2 &, Vec2 &);

		size_t insertTriangleRing(size_t, size_t, int, unsigned *);
		void addTriangleRing(size_t, size_t, int, int);
		void addTriangle(int, int, int, int);
		void initBuffer();
		void updateSegments();
	};
}

#endif

/* Copyright 2022 Floris Creyf
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

#ifndef PG_MESH_FORK_H
#define PG_MESH_FORK_H

#include "mesh.h"
#include "../math/intersection.h"
#include <vector>
#include <map>

namespace pg {
	class Fork {
	public:
		struct Section {
			int length;
			int edge1;
			int edge2;
			int offset;
		};

		Fork(Mesh &);
		bool isValidFork(Stem *, Stem *[2]);
		size_t getForkIndexCount(const Stem *);
		void reserveForkSpace(const Stem *, int );
		void reserveExtraTriangles(Section, Mesh::State &);
		Section getMiddle(Stem *[2], Mesh::State &);
		void connectForks(Stem *[2], Mesh::Segment [2], Mesh::State [2],
			Mesh::State &, Section);

	private:
		Mesh &mesh;

		size_t getForkVertexCount(const Stem *);
		void setBaseUVs(Stem *, int, int, DVertex *);
		void addExtraTriangles(
			std::vector<unsigned> &, Section &, int,
			size_t, size_t, Mesh::Segment &);
		void addForkTriangles(const Mesh::State &,
			const Mesh::State [2], Stem *[2],
			const Mesh::Segment [2]);
		int getForkMidpoint(int, Vec3, Vec3, Vec3, Quat);
		void setForkUVs(Stem *, int, int, DVertex *);
	};
}

#endif

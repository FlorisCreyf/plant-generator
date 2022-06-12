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

#ifndef PG_MESH_COLLAR_H
#define PG_MESH_COLLAR_H

#include "mesh.h"
#include "../math/intersection.h"
#include <vector>

namespace pg {
	class Collar {
	public:
		Collar(Plant *, Mesh &);
		void connectCollar(const Mesh::State &, bool);
		size_t insertCollar(Mesh::Segment, Mesh::Segment, size_t);
		void reserveBranchCollarSpace(Stem *, int);

	private:
		Plant *plant;
		Mesh &mesh;

		size_t getBranchCollarSize(Stem *);
		Mat4 getBranchCollarScale(Stem *, Stem *);
		Vec3 getSurfaceNormal(Vec3, Vec3, Vec3, Vec3, Vec3, Vec3, Vec3);
		DVertex moveToForkSurface(DVertex, Ray, Mesh::Segment);
		size_t getTriangleOffset(
			const Mesh::Segment &, const Mesh::Segment &);
		void insertCurve(Vec3 [4], int, DVertex, int, int, DVertex *);
		DVertex moveToSurface(DVertex, Ray, Mesh::Segment, size_t);
		void setBranchCollarNormals(size_t, size_t, int, int, int);
		void setBranchCollarUVs(size_t, Stem *, int, int, int);
	};
};

#endif

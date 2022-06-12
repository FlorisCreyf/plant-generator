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

#ifndef PG_MESH_GENERATOR_H
#define PG_MESH_GENERATOR_H

#include "../plant.h"
#include "mesh.h"
#include "collar.h"
#include "fork.h"

namespace pg {
	class MeshGenerator {
	public:
		MeshGenerator(Plant *plant);
		const Mesh &generate();
		const Mesh &getMesh();

	private:
		Plant *plant;
		Collar collarGenerator;
		Fork forkGenerator;
		Mesh mesh;

		Mesh::Segment addStem(Stem *, Mesh::State &, Mesh::State, bool);
		void addChildStems(Stem *, Stem *[2], Mesh::State &);
		void capStem(Stem *, int , size_t);
		void addSections(Mesh::State &, Mesh::Segment, bool, Stem *);
		void addSection(Mesh::State &, Quat, const CrossSection &);
		Quat rotateSection(Mesh::State &);
		void setInitialRotation(Stem *, Mesh::State &);
		float getTextureLength(Stem *, size_t);
		float getTextureLength(Stem *, size_t, size_t);

		void createBranchCollar(Mesh::State &, Mesh::Segment);
		void createFork(Stem *[2], Mesh::State &);
		void addForkBaseSection(Stem *, Stem *, Mesh::State &);
		void addForkSection(Stem *, Mesh::State &);

		void addLeaves(Stem *, const Mesh::State &);
		void addLeaf(Stem *, unsigned, const Mesh::State &);
		Geometry transformLeaf(const Leaf *, const Stem *);

		void setJointInfo(const Stem *, float, size_t, Vec2 &, Vec2 &);
		void updateJointState(Mesh::State &, Vec2 &, Vec2 &);
		void incrementJoint(Mesh::State &, const std::vector<Joint> &);
		void setInitialJointState(Mesh::State &, const Mesh::State &);
		std::pair<size_t, Joint> getJoint(float, const Stem *);
	};
}

#endif

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

#ifndef PG_STEM_H
#define PG_STEM_H

#include "leaf.h"
#include "math/math.h"
#include "math/curve.h"
#include "path.h"
#include "joint.h"
#include <vector>

#ifdef PG_SERIALIZE
#include <boost/archive/text_oarchive.hpp>
#endif

namespace pg {
	class Stem {
		friend class Plant;
		friend class StemPool;

		bool unused;
		union {
			Stem *nextAvailable;
			Stem *nextSibling;
		};
		union {
			Stem *prevSibling;
			Stem *prevAvailable;
		};
		Stem *child;
		Stem *parent;

		std::vector<Leaf> leaves;
		std::vector<Joint> joints;

		bool custom;
		int depth;
		float position;
		Vec3 location;
		Path path;
		int resolution;
		long material[2];
		Vec2 swelling;

		void updatePositions(Stem *stem);
		void copy(const Stem &stem);
		void init(Stem *parent = nullptr);

		#ifdef PG_SERIALIZE
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive &ar, const unsigned int version)
		{
			(void)version;
			ar & nextSibling;
			ar & prevSibling;
			ar & child;
			ar & parent;
			ar & depth;
			ar & leaves;
			ar & path;
			ar & resolution;
			ar & position;
			ar & location;
			ar & material;
			ar & swelling;
			ar & joints;
			ar & custom;
		}
		#endif

	public:
		enum Type {Outer, Inner};

		Stem(Stem *parent = nullptr);
		Stem(const Stem &original);
		~Stem();

		Stem &operator=(const Stem &stem);
		bool operator==(const Stem &stem) const;
		bool operator!=(const Stem &stem) const;

		void setCustom(bool custom);
		bool isCustom() const;

		size_t addLeaf(const Leaf &leaf);
		void insertLeaf(const Leaf &leaf, size_t index);
		size_t getLeafCount() const;
		Leaf *getLeaf(size_t index);
		const Leaf *getLeaf(size_t index) const;
		const std::vector<Leaf> &getLeaves() const;
		void removeLeaf(size_t index);

		void setResolution(int resolution);
		int getResolution() const;
		void setPath(Path &path);
		Path getPath() const;
		void setSwelling(Vec2 scale);
		Vec2 getSwelling() const;
		Vec2 getLimitedSwelling(float limit=1.0f) const;
		void setPosition(float position);
		float getPosition() const;
		Vec3 getLocation() const;
		void setMaterial(int feature, long material);
		long getMaterial(int feature) const;

		std::vector<Joint> getJoints() const;
		bool hasJoints() const;
		void addJoint(Joint joint);
		void clearJoints();

		Stem *getParent();
		const Stem *getParent() const;
		Stem *getSibling();
		const Stem *getSibling() const;
		Stem *getChild();
		const Stem *getChild() const;

		bool isDescendantOf(Stem *stem) const;
		int getDepth() const;
	};
}

#endif /* PG_STEM_H */

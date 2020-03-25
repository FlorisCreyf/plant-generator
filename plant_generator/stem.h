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
#include <map>

#ifdef PG_SERIALIZE
#include <boost/archive/text_oarchive.hpp>
#endif

namespace pg {
	class Stem {
		friend class Plant;

		static long counter;
		long id;

		Stem *nextSibling;
		Stem *prevSibling;
		Stem *child;
		Stem *parent;
		std::map<long, Leaf> leaves;

		int depth;
		float position;
		Vec3 location;
		Path path;
		int resolution = 10;
		long material[2] = {0};
		Vec2 swelling;

		void updatePositions(Stem *stem);
		void copy(const Stem &stem);

		#ifdef PG_SERIALIZE
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive &ar, const unsigned int version)
		{
			(void)version;
			ar & counter;
			ar & id;
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
			try {
				/* This might be changed so that the scaling
				is dependent on the angle between the parent
				and child stem. */
				ar & swelling;
			} catch (std::exception) {}
		}
		#endif

	public:
		enum {Outer, Inner};

		Stem(Stem *parent = nullptr);
		Stem(const Stem &original);
		~Stem();

		Stem &operator=(const Stem &stem);
		bool operator==(const Stem &stem) const;
		bool operator!=(const Stem &stem) const;

		long getID() const;

		int addLeaf(const Leaf &leaf);
		int getLeafCount();
		Leaf *getLeaf(long id);
		const std::map<long, Leaf> &getLeaves();
		void removeLeaf(long id);

		void setResolution(int resolution);
		int getResolution() const;
		void setPath(Path &path);
		Path getPath();
		void setSwelling(Vec2 scale);
		Vec2 getSwelling() const;
		void setPosition(float position);
		float getPosition() const;
		Vec3 getLocation() const;
		void setMaterial(int feature, long material);
		long getMaterial(int feature) const;

		Stem *getParent();
		Stem *getSibling();
		Stem *getChild();

		bool isDescendantOf(Stem *stem) const;
		int getDepth() const;
	};
}

#endif /* PG_STEM_H */

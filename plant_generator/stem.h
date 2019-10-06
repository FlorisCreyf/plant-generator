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
#include <boost/archive/text_oarchive.hpp>
#include <map>

namespace pg {
	class Stem {
		friend class Plant;
		friend class boost::serialization::access;

		static unsigned counter;
		unsigned id;

		Stem *nextSibling;
		Stem *prevSibling;
		Stem *child;
		Stem *parent;
		int depth;
		std::map<int, Leaf> leaves;
		Path path;
		int resolution = 10;
		float position;
		Vec3 location;
		unsigned material[2] = {0};

		void updatePositions(Stem *stem);

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
		}

	public:
		enum {Outer, Inner};

		Stem(Stem *parent = nullptr);
		bool operator==(const Stem &stem) const;
		bool operator!=(const Stem &stem) const;

		unsigned getId() const;

		int addLeaf(const Leaf &leaf);
		int getLeafCount();
		Leaf *getLeaf(int id);
		const std::map<int, Leaf> &getLeaves();
		void removeLeaf(int id);

		void setResolution(int resolution);
		int getResolution() const;
		void setPath(Path &path);
		Path getPath();
		void setPosition(float position);
		float getPosition() const;
		Vec3 getLocation() const;
		void setMaterial(int feature, unsigned material);
		unsigned getMaterial(int feature) const;

		Stem *getParent();
		Stem *getSibling();
		Stem *getChild();

		bool isDescendantOf(Stem *stem) const;
		int getDepth() const;
	};
}

#endif /* PG_STEM_H */

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
#include "volumetric_path.h"
#include <boost/archive/text_oarchive.hpp>
#include <vector>

namespace pg {
	class Stem {
		friend class Plant;
		friend class boost::serialization::access;

		unsigned long id;
		Stem *nextSibling;
		Stem *prevSibling;
		Stem *child;
		Stem *parent;
		int depth;
		std::vector<Leaf> leaves;
		VolumetricPath path;
		int resolution = 10;
		float position;
		Vec3 location;

		void updatePositions(Stem *stem);

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
		}

	public:
		Stem(Stem *parent = nullptr);
		bool operator==(const Stem &stem) const;
		bool operator!=(const Stem &stem) const;

		int addLeaf(const Leaf &leaf);
		int getLeafCount();
		Leaf *getLeaf(int index);
		void removeLeaf(int index);

		void setResolution(int resolution);
		int getResolution() const;
		void setPath(VolumetricPath &path);
		VolumetricPath getPath();
		void setPosition(float position);
		float getPosition() const;
		Vec3 getLocation() const;

		Stem *getParent();
		Stem *getSibling();
		Stem *getChild();

		bool isDescendantOf(Stem *stem) const;
		int getDepth() const;
	};
}

#endif /* PG_STEM_H */

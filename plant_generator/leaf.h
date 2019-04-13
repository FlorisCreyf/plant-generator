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

#ifndef PG_LEAF_H
#define PG_LEAF_H

#include "math/math.h"
#include <boost/archive/text_oarchive.hpp>

namespace pg {
	class Leaf {
		friend class boost::serialization::access;

		static unsigned counter;
		unsigned id;

		float position;
		Vec3 scale;
		unsigned material;
		unsigned mesh;
		Quat rotation;

		template<class Archive>
		void serialize(Archive &ar, const unsigned int version)
		{
			(void)version;
			ar & counter;
			ar & id;
			ar & position;
			ar & scale;
			ar & material;
			ar & mesh;
			ar & rotation;
		}

	public:
		Leaf();
		bool operator==(const Leaf &leaf) const;
		bool operator!=(const Leaf &leaf) const;

		unsigned getId() const;
		void setPosition(float position);
		float getPosition() const;
		void setRotation(Quat rotation);
		Quat getRotation();
		Quat getDefaultOrientation(Vec3 stemDirection);
		void setScale(Vec3 scale);
		Vec3 getScale() const;
		void setMaterial(unsigned material);
		unsigned getMaterial() const;
		void setMesh(unsigned mesh);
		unsigned getMesh() const;
	};
}

#endif /* PG_LEAF_H */

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

#include "path.h"
#include "parameter_tree.h"
#include "math/vec3.h"
#include "math/quat.h"

#ifdef PG_SERIALIZE
#include <boost/archive/text_oarchive.hpp>
#endif

namespace pg {
	class Leaf {
		bool custom;
		unsigned material;
		unsigned mesh;
		float position;
		Vec3 scale;
		Quat rotation;

#ifdef PG_SERIALIZE
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive &ar, const unsigned)
		{
			ar & position;
			ar & scale;
			ar & material;
			ar & mesh;
			ar & rotation;
			ar & custom;
		}
#endif

	public:
		Leaf();
		bool operator==(const Leaf &leaf) const;
		bool operator!=(const Leaf &leaf) const;

		void setCustom(bool custom);
		bool isCustom() const;
		void setPosition(float position);
		float getPosition() const;
		void setRotation(const LeafData &data, float position,
			const Path &path, int index);
		void setRotation(Quat rotation);
		Quat getRotation() const;
		void setScale(Vec3 scale);
		Vec3 getScale() const;
		void setMaterial(unsigned material);
		unsigned getMaterial() const;
		void setMesh(unsigned mesh);
		unsigned getMesh() const;
	};
}

#endif

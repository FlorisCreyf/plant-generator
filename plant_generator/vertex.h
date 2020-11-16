/* Copyright 2019 Floris Creyf
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

#ifndef PG_VERTEX_H
#define PG_VERTEX_H

#include "math/vec3.h"
#include "math/vec2.h"

#ifdef PG_SERIALIZE
#include <boost/archive/text_oarchive.hpp>
#endif

namespace pg {
	struct SVertex {
		Vec3 position;
		Vec3 normal;
		Vec2 uv;
	};

	struct DVertex {
		Vec3 position;
		Vec3 normal;
		Vec3 tangent;
		float tangentScale;
		Vec2 uv;
		Vec2 indices;
		Vec2 weights;

		#ifdef PG_SERIALIZE
		template<class Archive>
		void serialize(Archive &ar, const unsigned)
		{
			ar & position;
			ar & normal;
			ar & tangent;
			ar & tangentScale;
			ar & uv;
			ar & weights;
			ar & indices;
		}
		#endif
	};
}

#endif

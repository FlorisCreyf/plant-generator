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

#ifndef PG_VEC4_H
#define PG_VEC4_H

#include "vec3.h"

#ifdef PG_SERIALIZE
#include <boost/archive/text_oarchive.hpp>
#endif

namespace pg {
	struct Vec4 {
		float x;
		float y;
		float z;
		float w;

		float &operator[](unsigned column)
		{
			switch (column) {
				case 0: return x;
				case 1: return y;
				case 2: return z;
				case 3: return w;
				default: return w;
			}
		}

		const float &operator[](unsigned column) const
		{
			switch (column) {
				case 0: return x;
				case 1: return y;
				case 2: return z;
				case 3: return w;
				default: return w;
			}
		}

		#ifdef PG_SERIALIZE
		template<class Archive>
		void serialize(Archive &ar, const unsigned int version)
		{
			(void)version;
			ar & x;
			ar & y;
			ar & z;
			ar & w;
		}
		#endif
	};

	inline Vec4 toVec4(const Vec3 &vec, const float w)
	{
		Vec4 v = {vec.x, vec.y, vec.z, w};
		return v;
	}

	inline Vec3 toVec3(const Vec4 &vec)
	{
		Vec3 v = {vec.x, vec.y, vec.z};
		return v;
	}

	inline bool operator==(const Vec4 &a, const Vec4 &b)
	{
		return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
	}

	inline bool operator!=(const Vec4 &a, const Vec4 &b)
	{
		return !(a == b);
	}

	float magnitude(Vec4 vec);
	Vec4 normalize(Vec4 vec);
}

#endif

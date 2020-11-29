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
#include <iostream>
#include <iomanip>

#ifdef PG_SERIALIZE
#include <boost/archive/text_oarchive.hpp>
#endif

namespace pg {
	struct Vec4 {
		float x;
		float y;
		float z;
		float w;

		Vec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f)
		{

		}

		Vec4(float x, float y, float z, float w) :
			x(x), y(y), z(z), w(w)
		{

		}

		Vec4 &operator+=(const Vec4 &b)
		{
			this->x += b.x;
			this->y += b.y;
			this->z += b.z;
			this->w += b.w;
			return *this;
		}

		Vec4 &operator-=(const Vec4 &b)
		{
			this->x -= b.x;
			this->y -= b.y;
			this->z -= b.z;
			this->w -= b.w;
			return *this;
		}

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
		return Vec4(vec.x, vec.y, vec.z, w);
	}

	inline Vec3 toVec3(const Vec4 &vec)
	{
		return Vec3(vec.x, vec.y, vec.z);
	}

	inline bool operator==(const Vec4 &a, const Vec4 &b)
	{
		return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
	}

	inline bool operator!=(const Vec4 &a, const Vec4 &b)
	{
		return !(a == b);
	}

	inline std::ostream &operator<<(std::ostream &stream, const Vec4 &v)
	{
		stream << "(" << v.x << ", " << v.y << ", " << v.z << ", ";
		stream << v.w << ")";
		return stream;
	}

	inline Vec4 operator+(const Vec4 &a, const Vec4 &b)
	{
		Vec4 vec = a;
		vec += b;
		return vec;
	}

	inline Vec4 operator-(const Vec4 &a, const Vec4 &b)
	{
		Vec4 vec = a;
		vec -= b;
		return vec;
	}

	float magnitude(Vec4 vec);
	Vec4 normalize(Vec4 vec);
}

#endif

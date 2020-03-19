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

#ifndef PG_VEC3_H
#define PG_VEC3_H

#include "vec2.h"
#include <cmath>
#include <iostream>
#include <iomanip>

#ifdef PG_SERIALIZE
#include <boost/archive/text_oarchive.hpp>
#endif

namespace pg {
	struct Vec3 {
		float x;
		float y;
		float z;

		Vec3 &operator+=(const Vec3 &b)
		{
			this->x += b.x;
			this->y += b.y;
			this->z += b.z;
			return *this;
		}

		Vec3 &operator-=(const Vec3 &b)
		{
			this->x -= b.x;
			this->y -= b.y;
			this->z -= b.z;
			return *this;
		}

		Vec3 &operator*=(float n)
		{
			this->x *= n;
			this->y *= n;
			this->z *= n;
			return *this;
		}

		Vec3 &operator/=(float n)
		{
			this->x /= n;
			this->y /= n;
			this->z /= n;
			return *this;
		}

		#ifdef PG_SERIALIZE
		template<class Archive>
		void serialize(Archive &ar, const unsigned int version) {
			(void)version;
			ar & x;
			ar & y;
			ar & z;
		}
		#endif
	};

	inline pg::Vec3 getZeroVec3()
	{
		Vec3 v;
		v.x = v.y = v.z = 0.0f;
		return v;
	}

	inline Vec2 toVec2(const Vec3 &vec)
	{
		Vec2 v = {vec.x, vec.z};
		return v;
	}

	inline bool isZero(const Vec3 &vec)
	{
		return vec.x == 0 && vec.y == 0 && vec.z == 0;
	}

	inline std::ostream &operator<<(std::ostream &stream, const Vec3 &v)
	{
		stream << "(" << v.x << ", " << v.y << ", " << v.z << ")";
		return stream;
	}

	inline bool operator==(const Vec3 &a, const Vec3 &b)
	{
		return a.x == b.x && a.y == b.y && a.z == b.z;
	}

	inline bool operator!=(const Vec3 &a, const Vec3 &b)
	{
		return !(a == b);
	}

	inline Vec3 operator+(const Vec3 &a, const Vec3 &b)
	{
		Vec3 vec = a;
		vec += b;
		return vec;
	}

	inline Vec3 operator-(const Vec3 &a, const Vec3 &b)
	{
		Vec3 vec = a;
		vec -= b;
		return vec;
	}

	inline Vec3 operator*(float a, const Vec3 &b)
	{
		Vec3 vec = b;
		vec *= a;
		return vec;
	}

	inline Vec3 operator/(const Vec3 &a, float b)
	{
		Vec3 vec = a;
		vec /= b;
		return vec;
	}

	float angle(Vec3 a, Vec3 b);
	Vec3 cross(Vec3 a, Vec3 b);
	float dot(Vec3 a, Vec3 b);
	float magnitude(Vec3 vec);
	Vec3 normalize(Vec3 vec);
	/** Projects a onto b and returns distance along b. */
	float project(Vec3 a, Vec3 b);
	Vec3 projectOntoPlane(Vec3 vec, Vec3 normal);
	Vec3 rotateAroundAxis(Vec3 vec, Vec3 axis, float n);
	Vec3 lerp(Vec3 a, Vec3 b, float t);
}

#endif

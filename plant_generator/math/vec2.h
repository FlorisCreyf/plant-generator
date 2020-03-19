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

#ifndef PG_VEC2_H
#define PG_VEC2_H

#include <iostream>
#include <iomanip>

#ifdef PG_SERIALZE
#include <boost/archive/text_oarchive.hpp>
#endif

namespace pg {
	struct Vec2 {
		float x;
		float y;

		Vec2 &operator+=(const Vec2 &b)
		{
			this->x += b.x;
			this->y += b.y;
			return *this;
		}

		Vec2 &operator-=(const Vec2 &b)
		{
			this->x -= b.x;
			this->y -= b.y;
			return *this;
		}

		Vec2 &operator*=(float n)
		{
			this->x *= n;
			this->y *= n;
			return *this;
		}

		#ifdef PG_SERIALIZE
		template<class Archive>
		void serialize(Archive &ar, const unsigned int version)
		{
			(void)version;
			ar & x;
			ar & y;
		}
		#endif
	};

	inline bool operator==(const Vec2 &a, const Vec2 &b)
	{
		return a.x == b.x && a.y == b.y;
	}

	inline bool operator!=(const Vec2 &a, const Vec2 &b)
	{
		return !(a == b);
	}

	inline std::ostream &operator<<(std::ostream &stream, const Vec2 &v)
	{
		stream << "(" << v.x << ", " << v.y << ")";
		return stream;
	}

	inline Vec2 operator+(const Vec2 &a, const Vec2 &b)
	{
		Vec2 vec = a;
		vec += b;
		return vec;
	}

	inline Vec2 operator-(const Vec2 &a, const Vec2 &b)
	{
		Vec2 vec = a;
		vec -= b;
		return vec;
	}

	inline Vec2 operator*(float a, const Vec2 &b)
	{
		Vec2 vec = b;
		vec *= a;
		return vec;
	}

	float dot(Vec2 a, Vec2 b);
	float magnitude(Vec2 vec);
	Vec2 normalize(Vec2 vec);
	Vec2 perp(Vec2 vec);
}

#endif

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

#ifndef PG_QUAT_H
#define PG_QUAT_H

#include "mat4.h"
#include "vec3.h"

#ifdef PG_SERIALIZE
#include <boost/archive/text_oarchive.hpp>
#endif

namespace pg {
	struct Quat {
		float x;
		float y;
		float z;
		float w;

		Quat &operator*=(const Quat &b)
		{
			Vec3 m = {this->x, this->y, this->z};
			Vec3 n = {b.x, b.y, b.z};
			float s = this->w * b.w - dot(m, n);
			Vec3 f = (cross(m, n) + b.w * m) + this->w * n;
			this->x = f.x;
			this->y = f.y;
			this->z = f.z;
			this->w = s;
			return *this;
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

	inline Quat toQuat(const Vec3 &vec, const float w)
	{
		Quat q = {vec.x, vec.y, vec.z, w};
		return q;
	}

	inline Vec3 toVec3(const Quat &quat)
	{
		Vec3 q = {quat.x, quat.y, quat.z};
		return q;
	}

	inline Quat operator*(Quat q, float s)
	{
		q.x *= s;
		q.y *= s;
		q.z *= s;
		q.w *= s;
		return q;
	}

	inline Quat operator*(float s, Quat q)
	{
		return q * s;
	}

	inline Quat operator*(const Quat &a, const Quat &b)
	{
		Quat q = a;
		q *= b;
		return q;
	}

	inline bool operator==(const Quat &a, const Quat &b)
	{
		return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
	}

	inline bool operator!=(const Quat &a, const Quat &b)
	{
		return !(a == b);
	}

	Quat conjugate(Quat quat);
	Quat fromAxisAngle(Vec3 vec, float theta);
	Quat inverse(Quat quat);
	float norm(Quat quat);
	Quat normalize(Quat quat);
	Mat4 toMat4(Quat quat);
	Quat rotateIntoVecQ(Vec3 normal, Vec3 direction);
	Quat slerp(Quat a, Quat b, float t);
}

#endif

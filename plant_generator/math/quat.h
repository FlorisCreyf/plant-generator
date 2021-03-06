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
#include "vec4.h"

#ifdef PG_SERIALIZE
#include <boost/archive/text_oarchive.hpp>
#endif

namespace pg {
	struct Quat {
		float x;
		float y;
		float z;
		float w;

		Quat() : x(0.0f), y(0.0f), z(0.0f), w(0.0f)
		{

		}

		Quat(float x, float y, float z, float w) :
			x(x), y(y), z(z), w(w)
		{

		}

		Quat &operator+=(const Quat &b)
		{
			this->x += b.x;
			this->y += b.y;
			this->z += b.z;
			this->w += b.w;
			return *this;
		}

		Quat &operator*=(const Quat &b)
		{
			Vec3 m(this->x, this->y, this->z);
			Vec3 n(b.x, b.y, b.z);
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
		void serialize(Archive &ar, const unsigned)
		{
			ar & x;
			ar & y;
			ar & z;
			ar & w;
		}
#endif
	};

	inline Quat toQuat(const Vec4 &vec)
	{
		return Quat(vec.x, vec.y, vec.z, vec.w);
	}

	inline Vec4 toVec4(const Quat &quat)
	{
		return Vec4(quat.x, quat.y, quat.z, quat.w);
	}

	inline Quat toQuat(const Vec3 &vec, const float w)
	{
		return Quat(vec.x, vec.y, vec.z, w);
	}

	inline Vec3 toVec3(const Quat &quat)
	{
		return Vec3(quat.x, quat.y, quat.z);
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

	inline Quat operator+(const Quat &a, const Quat &b)
	{
		Quat q = a;
		q += b;
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

	inline std::ostream &operator<<(std::ostream &stream, const Quat &q)
	{
		stream << "(" << q.x << ", " << q.y << ", " << q.z << ", ";
		stream << q.w << ")";
		return stream;
	}

	inline Quat conjugate(Quat quat)
	{
		quat.x = -quat.x;
		quat.y = -quat.y;
		quat.z = -quat.z;
		return quat;
	}

	inline Vec3 rotate(Quat rotation, Vec3 point)
	{
		Quat q = toQuat(point, 0.0f);
		return toVec3(rotation * q * conjugate(rotation));
	}

	Quat fromAxisAngle(Vec3 vec, float theta);
	Quat inverse(Quat quat);
	float norm(Quat quat);
	Quat normalize(Quat quat);
	Mat4 toMat4(Quat quat);
	Quat rotateIntoVecQ(Vec3 normal, Vec3 direction);
	Quat nlerp(Quat a, Quat b, float t);
	Quat slerp(Quat a, Quat b, float t);
	Quat toBasis(Vec3 i, Vec3 j, Vec3 k);
}

#endif

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

#ifndef PG_MATH_H
#define PG_MATH_H

#include <iostream>
#include <iomanip>

namespace pg {
	struct Vec2 {
		float x;
		float y;

		Vec2 &operator +=(const Vec2 &b)
		{
			this->x += b.x;
			this->y += b.y;
			return *this;
		}

		Vec2 &operator -=(const Vec2 &b)
		{
			this->x -= b.x;
			this->y -= b.y;
			return *this;
		}

		Vec2 &operator *=(float n)
		{
			this->x *= n;
			this->y *= n;
			return *this;
		}
	};

	struct Vec3 {
		float x;
		float y;
		float z;

		Vec3 &operator +=(const Vec3 &b)
		{
			this->x += b.x;
			this->y += b.y;
			this->z += b.z;
			return *this;
		}

		Vec3 &operator -=(const Vec3 &b)
		{
			this->x -= b.x;
			this->y -= b.y;
			this->z -= b.z;
			return *this;
		}

		Vec3 &operator *=(float n)
		{
			this->x *= n;
			this->y *= n;
			this->z *= n;
			return *this;
		}
	};

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
	};

	struct Mat4 {
		Vec4 vectors[4];

		Vec4 &operator[](unsigned row)
		{
			return vectors[row];
		}

		const Vec4 &operator[](unsigned row) const
		{
			return vectors[row];
		}
	};

	typedef Vec4 Quat;

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

	inline Vec2 toVec2(const Vec3 &vec)
	{
		Vec2 v = {vec.x, vec.z};
		return v;
	}

	inline bool isZero(const Vec3 &vec)
	{
		return vec.x == 0 && vec.y == 0 && vec.z == 0;
	}

	float abs(float value);
	Vec2 perp(const Vec2 &vec);
	float dot(const Vec2 &a, const Vec2 &b);
	float dot(const Vec3 &a, const Vec3 &b);
	Vec3 cross(const Vec3 &a, const Vec3 &b);
	float magnitude(const Vec2 &vec);
	float magnitude(const Vec3 &vec);
	Vec2 normalize(const Vec2 &vec);
	Vec3 normalize(const Vec3 &vec);
	Quat normalize(const Quat &quat);
	Mat4 transpose(const Mat4 &mat);
	Mat4 rotateIntoVec(const Vec3 &normal, const Vec3 &direction);
	Vec3 rotateAroundAxis(const Vec3 &vec, const Vec3 &axis, float n);
	Mat4 translate(const Vec3 &vec);
	Mat4 rotateXY(float x, float y);
	Quat fromAxisAngle(const Vec3 &vec, float theta);
	Mat4 quatToMat4(const Quat &quat);
	Quat slerp(const Quat &a, const Quat &b, float t);
	Mat4 identity();
	/** Projects a onto b and returns distance along b. */
	float project(Vec3 &a, Vec3 &b);

	inline std::ostream &operator<<(std::ostream &stream, const Vec2 &v)
	{
		stream << "(" << v.x << ", " << v.y << ")";
		return stream;
	}

	inline std::ostream &operator<<(std::ostream &stream, const Vec3 &v)
	{
		stream << "(" << v.x << ", " << v.y << ", " << v.z << ")";
		return stream;
	}

	inline std::ostream &operator<<(std::ostream &stream, const Mat4 &m)
	{
		stream << std::fixed;
		stream << "[" << std::setw(10) << m[0].x << " " <<
			std::setw(10) << m[0].y << " " <<
			std::setw(10) << m[0].z << " " <<
			std::setw(10) << m[0].w << "]\n";
		stream << "[" << std::setw(10) << m[1].x << " " <<
			std::setw(10) << m[1].y << " " <<
			std::setw(10) << m[1].z << " " <<
			std::setw(10) << m[1].w << "]\n";
		stream << "[" << std::setw(10) << m[2].x << " " <<
			std::setw(10) << m[2].y << " " <<
			std::setw(10) << m[2].z << " " <<
			std::setw(10) << m[2].w << "]\n";
		stream << "[" << std::setw(10) << m[3].x << " " <<
			std::setw(10) << m[3].y << " " <<
			std::setw(10) << m[3].z << " " <<
			std::setw(10) << m[3].w << "]\n";
		stream << std::defaultfloat;
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

	inline Mat4 operator*(const Mat4 &a, const Mat4 &b)
	{
		int row = 0;
		int col = 0;
		Mat4 m;
		for (row = 0; row < 4; row++)
			for (col = 0; col < 4; col++)
				m[row][col] =
					a[0][col]*b[row][0] +
					a[1][col]*b[row][1] +
					a[2][col]*b[row][2] +
					a[3][col]*b[row][3];
		return m;
	}

	inline Vec4 operator*(const Mat4 &a, const Vec4 &b)
	{
		Vec4 v;
		v.x = b.x*a[0][0] + b.y*a[1][0] + b.z*a[2][0] + a[3][0]*b.w;
		v.y = b.x*a[0][1] + b.y*a[1][1] + b.z*a[2][1] + a[3][1]*b.w;
		v.z = b.x*a[0][2] + b.y*a[1][2] + b.z*a[2][2] + a[3][2]*b.w;
		v.w = b.x*a[0][3] + b.y*a[1][3] + b.z*a[2][3] + a[3][3]*b.w;
		return v;
	}

	inline Quat operator*(const Quat &a, const Quat &b)
	{
		float s = a.w * b.w - dot(toVec3(a), toVec3(b));
		Vec3 x = cross(toVec3(a), toVec3(b));
		Vec3 y = b.w * toVec3(a);
		Vec3 z = a.w * toVec3(b);
		Vec3 f = (x + y) + z;
		return toVec4(f, s);
	}
}

#endif /* PG_MATH_H */

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

#ifndef PG_MAT4_H
#define PG_MAT4_H

#include "vec4.h"

namespace pg {
	struct Mat4 {
		Vec4 vectors[4];

		Mat4()
		{

		}

		Mat4(
			float a11, float a12, float a13, float a14,
			float a21, float a22, float a23, float a24,
			float a31, float a32, float a33, float a34,
			float a41, float a42, float a43, float a44)
		{
			this->vectors[0] = Vec4(a11, a12, a13, a14);
			this->vectors[1] = Vec4(a21, a22, a23, a24);
			this->vectors[2] = Vec4(a31, a32, a33, a34);
			this->vectors[3] = Vec4(a41, a42, a43, a44);
		}

		Vec4 &operator[](unsigned row)
		{
			return vectors[row];
		}

		const Vec4 &operator[](unsigned row) const
		{
			return vectors[row];
		}

		Mat4 &operator*=(Mat4 b)
		{
			Mat4 a = *this;
			for (int row = 0; row < 4; row++) {
				for (int col = 0; col < 4; col++) {
					float r = 0;
					r += a[0][col]*b[row][0];
					r += a[1][col]*b[row][1];
					r += a[2][col]*b[row][2];
					r += a[3][col]*b[row][3];
					this->vectors[row][col] = r;
				}
			}
			return *this;
		}

		Vec3 apply(const Vec3 vec, float w) const;
		Mat4 transpose() const;
	};

	inline Mat4 operator*(const Mat4 &a, const Mat4 &b)
	{
		Mat4 result = a;
		result *= b;
		return result;
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

	Mat4 identity();
	Mat4 rotateIntoVec(const Vec3 &normal, const Vec3 &direction);
	Mat4 rotateXY(float x, float y);
	Mat4 rotateZYX(float z, float y, float x);
	Mat4 scale(const Vec3 &vec);
	Mat4 translate(const Vec3 &vec);
	Mat4 transpose(const Mat4 &mat);
}

#endif

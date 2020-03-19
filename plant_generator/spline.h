/* Copyright 2017-2018 Floris Creyf
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

#ifndef PG_SPLINE_H
#define PG_SPLINE_H

#include "math/math.h"
#include <vector>
#include <set>

#ifdef PG_SERIALIZE
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#endif

namespace pg {
	class Spline {
		void adjustCubic();
		void adjustLinear();
		std::vector<Vec3> controls;
		int degree = 3;
		int insertCubic(int index, pg::Vec3 point);
		void moveCubic(unsigned, Vec3, bool);
		void removeCubic(unsigned);

		#ifdef PG_SERIALIZE
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive &ar, const unsigned int version)
		{
			(void)version;
			ar & controls;
			ar & degree;
		}
		#endif

	public:
		bool operator==(const Spline &spline) const;
		bool operator!=(const Spline &spline) const;
		void setControls(std::vector<Vec3> controls);
		void addControl(Vec3 control);
		std::vector<Vec3> getControls() const;
		int getSize() const;
		int getCurveCount() const;
		/** 1 = linear, 2 = quadratic, 3 = cubic, . . . */
		void setDegree(int degree);
		int getDegree() const;
		Vec3 getPoint(float t) const;
		Vec3 getPoint(int curve, float t) const;
		Vec3 getDirection(unsigned index);
		/** Returns the index of the center point of the insertion */
		int insert(unsigned index, Vec3 point);
		void remove(unsigned index);
		/* Changes the degree and modifies the spline. */
		void adjust(int degree);
		void move(unsigned index, Vec3 location, bool parallel);
		void parallelize(unsigned index);
		/** Flatten a cubic curve. */
		void linearize(int curve);
		void clear();
	};
}

#endif /* PG_SPLINE_H */

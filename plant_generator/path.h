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

#ifndef PG_PATH_H
#define PG_PATH_H

#include "math/math.h"
#include "spline.h"
#include <vector>

namespace pg {
	class Path {
	protected:
		std::vector<Vec3> path;
		Spline spline;
		int resolution = 2;
		int subdivisions = 0;
		Spline radius;
		float minRadius = 0.015f;
		float maxRadius = 0.2f;

		#ifdef PG_SERIALIZE
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive &ar, const unsigned int version)
		{
			(void)version;
			ar & path;
			ar & spline;
			ar & resolution;
			ar & subdivisions;
			ar & radius;
			ar & minRadius;
			ar & maxRadius;
		}
		#endif

	public:
		bool operator==(const Path &path) const;
		bool operator!=(const Path &path) const;

		void setSpline(Spline &spline);
		Spline getSpline();
		/** Sets the divisions for each curve in the path. */
		void setResolution(int resolution);
		int getResolution() const;
		void subdivide(int level);
		int getSubdivisions() const;
		/** Evaluate points along the spline. The first curve is
		always linear if linearStart is true. */
		void generate(bool linearStart=false);

		std::vector<Vec3> get() const;
		/** Gets a point on the path. */
		Vec3 get(const int index) const;
		/** Gets the number of point on the path. */
		int getSize() const;
		/** Gets a point at a distance along the path. */
		Vec3 getIntermediate(float distance) const;
		/** Get the length of the path. */
		float getLength() const;
		/* Get the direction of a line segment of the path. */
		Vec3 getDirection(size_t index) const;
		Vec3 getAverageDirection(size_t index) const;
		Vec3 getIntermediateDirection(float t) const;
		/* Get the distance along the path to a control point. */
		float getDistance(int index) const;
		/* Get the distance between control points. */
		float getIntermediateDistance(int index) const;

		void setMaxRadius(float radius);
		float getMaxRadius();
		void setMinRadius(float radius);
		float getMinRadius();
		void setRadius(Spline spline);
		Spline getRadius();
		float getRadius(int index);
		float getIntermediateRadius(float t);
	};
}

#endif /* PG_PATH_H */

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

#ifndef PG_VOLUMETRIC_PATH_H
#define PG_VOLUMETRIC_PATH_H

#include "path.h"

namespace pg {
	class VolumetricPath : public Path {
		friend class boost::serialization::access;

		Spline radius;
		float minRadius = 0.02f;
		float maxRadius = 0.2f;

		template<class Archive>
		void serialize(Archive &ar, const unsigned int version)
		{
			(void)version;
			ar & radius;
			ar & minRadius;
			ar & maxRadius;
			Path::serialize(ar, version);
		}

	public:
		void setMaxRadius(float radius);
		float getMaxRadius();
		void setMinRadius(float radius);
		float getMinRadius();
		void setRadius(Spline spline);
		Spline getRadius();
		float getRadius(int index);
	};
}

#endif /* PG_VOLUMETRIC_PATH_H */

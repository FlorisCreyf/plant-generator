/* Copyright 2020 Floris Creyf
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

#ifndef PG_ANIMATION_H
#define PG_ANIMATION_H

#include "plant.h"

#ifdef PG_SERIALIZE
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#endif

namespace pg {
	struct KeyFrame {
		float time;
		Quat rotation;
		Vec3 translation;
		Quat inverseRotation;
		Vec3 inverseTranslation;

		#ifdef PG_SERIALIZE
		template<class Archive>
		void serialize(Archive &ar, const unsigned int version)
		{
			(void)version;
			ar & time;
			ar & rotation;
			ar & translation;
			ar & inverseRotation;
			ar & inverseTranslation;
		}
		#endif /* PG_SERIALIZE */
	};

	struct Animation {
		std::vector<std::vector<KeyFrame>> frames;

		#ifdef PG_SERIALIZE
		template<class Archive>
		void serialize(Archive &ar, const unsigned int version)
		{
			(void)version;
			ar & frames;
		}
		#endif /* PG_SERIALIZE */
	};
}

#endif /* PG_ANIMATION_H */

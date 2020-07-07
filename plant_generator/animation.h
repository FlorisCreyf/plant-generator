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
		Quat rotation;
		Vec4 translation;
		Vec4 finalTranslation;

		#ifdef PG_SERIALIZE
		template<class Archive>
		void serialize(Archive &ar, const unsigned int)
		{
			ar & rotation;
			ar & translation;
			ar & finalTranslation;
		}
		#endif /* PG_SERIALIZE */
	};

	struct Animation {
		int timeStep;
		std::vector<std::vector<KeyFrame>> frames;

		std::vector<KeyFrame> getFrame(int ticks, Stem *stem);
		size_t getFrameCount() const;

		#ifdef PG_SERIALIZE
		template<class Archive>
		void serialize(Archive &ar, const unsigned int)
		{
			ar & frames;
		}
		#endif /* PG_SERIALIZE */

	private:
		std::vector<KeyFrame> mixedFrames;

		void createFrame(float, size_t, size_t, Stem *);
	};
}

#endif /* PG_ANIMATION_H */

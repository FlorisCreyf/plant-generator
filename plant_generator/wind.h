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

#ifndef PG_WIND_GENERATOR_H
#define PG_WIND_GENERATOR_H

#include "animation.h"
#include <vector>
#include <random>

#ifdef PG_SERIALIZE
#include <boost/archive/text_oarchive.hpp>
#endif

namespace pg {
	class Wind {
	public:
		Wind();
		void setSpeed(float speed);
		void setDirection(Vec3 direction);
		Animation generate(Plant *plant);

	private:
		Vec3 direction;
		float speed;
		int frameCount;
		std::mt19937 randomGenerator;

		void animateJoint(int, float, float, Vec3, Animation &);
		void setInverseTransform(int, Vec3, Vec3, Animation &);
		void transformJoint(Plant *, Stem *, Vec3, Animation &);
		void transformChildJoint(
			Stem *, unsigned, Plant *, Vec3, Animation &);
		int generateJoint(Stem *, int, int, size_t &);
		void initFrames(Stem *, size_t, Animation &);

		#ifdef PG_SERIALIZE
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive &ar, const unsigned int version)
		{
			(void)version;
			ar & direction;
			ar & speed;
		}
		#endif /* PG_SERIALIZE */
	};
}

#endif

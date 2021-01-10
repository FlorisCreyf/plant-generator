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
		void setSeed(int seed);
		int getSeed() const;
		void setFrameCount(int count);
		int getFrameCount() const;
		void setTimeStep(int step);
		int getTimeStep() const;
		int getDuration() const;
		void setDirection(Vec3 direction);
		Vec3 getDirection() const;
		void setResistance(float resistance);
		float getResistance() const;
		void setThreshold(float threshold);
		float getThreshold() const;
		Animation generate(Plant *plant);

	private:
		int seed;
		std::mt19937 mt;
		Vec3 direction;
		float speed;
		float resistance;
		float threshold;
		int timeStep;
		int frameCount;

		void setRotation(int, float, float, Vec3, Animation &);
		void setNoRotation(int, Animation &);
		void setTranslation(int, Vec3, Vec3, Animation &);
		void setRootTranslation(Stem *, Animation &);
		void transformJoint(Plant *, Stem *, Vec3, Animation &);
		void transformChildJoints(Stem *, int, Plant *, Vec3,
			Animation &);
		int generateJoint(Stem *, int, int, size_t &);
		int generateChildJoints(Stem *, int, int, size_t &, float);

#ifdef PG_SERIALIZE
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive &ar, const unsigned version)
		{
			ar & direction;
			ar & speed;
			if (version == 2) {
				ar & resistance;
				ar & threshold;
			}
			ar & timeStep;
			ar & frameCount;
			ar & seed;
		}
#endif
	};
}

#ifdef PG_SERIALIZE
BOOST_CLASS_VERSION(pg::Wind, 2)
#endif

#endif

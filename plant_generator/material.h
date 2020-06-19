/* Copyright 2018 Floris Creyf
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

#ifndef PG_MATERIAL_H
#define PG_MATERIAL_H

#include <string>

#ifdef PG_SERIALIZE
#include <boost/archive/text_oarchive.hpp>
#endif

namespace pg {
	class Material {
		std::string name;
		std::string texture;
		float ratio = 1.0f;

		#ifdef PG_SERIALIZE
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive &ar, const unsigned int version)
		{
			(void)version;
			ar & name;
			ar & texture;
			ar & ratio;
		}
		#endif

	public:
		void setName(const char *name);
		void setName(std::string name);
		std::string getName() const;
		void setTexture(const char *file);
		void setTexture(std::string file);
		std::string getTexture() const;
		void setRatio(float ratio);
		float getRatio() const;
	};
}

#endif /* PG_MATERIAL_H */

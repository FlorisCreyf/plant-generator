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
#include <boost/archive/text_oarchive.hpp>

namespace pg {
	class Material {
		friend class boost::serialization::access;

		static long counter;
		long id;
		std::string name;
		std::string texture;
		float ratio;

		template<class Archive>
		void serialize(Archive &ar, const unsigned int version)
		{
			(void)version;
			ar & counter;
			ar & id;
			ar & name;
			ar & texture;
			ar & ratio;
		}

	public:
		Material();
		long getID();
		void setName(const char *name);
		void setName(std::string name);
		std::string getName();
		void setTexture(const char *file);
		void setTexture(std::string file);
		std::string getTexture();
		void setRatio(float ratio);
		float getRatio();
	};
}

#endif /* PG_MATERIAL_H */

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
	public:
		enum Type {
			Albedo,
			Opacity,
			Normal,
			Specular,
			MapQuantity
		};

		Material();
		void setName(const char *name);
		void setName(std::string name);
		std::string getName() const;
		void setTexture(const char *file, int index);
		void setTexture(std::string file, int index);
		std::string getTexture(int index) const;
		void setRatio(float ratio);
		float getRatio() const;
		void setShininess(float shininess);
		float getShininess() const;
		void setAmbient(Vec3 ambient);
		Vec3 getAmbient() const;

	private:
		std::string name;
		std::string textures[MapQuantity];
		float ratio = 1.0f;
		float shininess;
		Vec3 ambient;

		#ifdef PG_SERIALIZE
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive &ar, const unsigned)
		{
			ar & name;
			ar & textures;
			ar & ratio;
			ar & shininess;
			ar & ambient;
		}
		#endif
	};
}

#endif /* PG_MATERIAL_H */

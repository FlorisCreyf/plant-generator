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

#ifndef PG_GEOMETRY_H
#define PG_GEOMETRY_H

#include "math/math.h"
#include "vertex.h"
#include <vector>
#include <string>
#include <boost/archive/text_oarchive.hpp>

namespace pg {
	class Geometry {
	public:
		Geometry();
		unsigned getId() const;
		std::string getName() const;
		void setName(std::string name);
		void setPlane();
		void setPerpendicularPlanes();
		void setPoints(std::vector<Vertex> points);
		void setIndices(std::vector<unsigned> indices);
		const std::vector<Vertex> &getPoints() const;
		const std::vector<unsigned> &getIndices() const;
		void transform(Quat rotation, Vec3 scale, Vec3 translation);
		void toCenter();
		void clear();

	private:
		friend class boost::serialization::access;

		std::vector<Vertex> points;
		std::vector<unsigned> indices;
		static unsigned counter;
		unsigned id;
		std::string name;

		template<class Archive>
		void serialize(Archive &ar, const unsigned int version)
		{
			(void)version;
			ar & counter;
			ar & id;
			ar & name;
			ar & points;
			ar & indices;
		}
	};
}

#endif /* PG_GEOMETRY_H */

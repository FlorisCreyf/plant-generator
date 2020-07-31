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

#ifndef PG_CROSS_SECTION_H
#define PG_CROSS_SECTION_H

#include "vertex.h"
#include "spline.h"
#include <vector>

namespace pg {
	class CrossSection {
		int resolution;
		Spline spline;
		std::vector<SVertex> vertices;

		void generateCircle();
		void generateSpline();

	public:
		int getResolution() const;
		void setSpline(Spline spline);
		void generate(int resolution);
		void scale(float x, float y);
		void setVertices(std::vector<SVertex> vertices);
		std::vector<SVertex> getVertices() const;
	};
}

#endif /* PG_CROSS_SECTION_H */

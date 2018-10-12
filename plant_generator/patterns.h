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

#ifndef PG_PATTERNS_H
#define PG_PATTERNS_H

#include "math.h"
#include <vector>

namespace pg {
	inline pg::Spline getDefaultCurve(unsigned type)
	{
		pg::Spline spline;
		std::vector<pg::Vec3> controls;

		switch (type) {
		case 0:
			controls.push_back({0.0f, 0.3f, 1.0f});
			controls.push_back({0.0f, 0.3f, 0.25f});
			controls.push_back({1.0f, 0.3f, 0.5f});
			controls.push_back({1.0f, 0.3f, 0.0f});

			spline.setControls(controls);
			spline.setDegree(3);
		}

		return spline;
	}
}

#endif /* PG_PATTERNS_H */

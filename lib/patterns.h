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

#ifndef TM_PATTERNS_H
#define TM_PATTERNS_H

#include "math.h"
#include <vector>

std::vector<Vec3> getDefaultCurve(unsigned type)
{
	std::vector<Vec3> curve;
	switch (type) {
	case 0:
		curve.push_back({0.0f, -0.3f, 1.0f});
		curve.push_back({0.0f, -0.3f, 0.75f});
		curve.push_back({0.05f, -0.3f, 0.75f});
		curve.push_back({0.175f, -0.3f, 0.75f});
		curve.push_back({0.175f, -0.3f, 0.75f});
		curve.push_back({0.3f, -0.3f, 0.75f});
		curve.push_back({0.75f, -0.3f, 0.25f});
		curve.push_back({1.0f, -0.3f, 0.0f});
	}
	return curve;
}

#endif /* TM_PATTERNS_H */

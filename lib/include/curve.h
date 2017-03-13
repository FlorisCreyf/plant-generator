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

#ifndef TM_CURVE_H
#define TM_CURVE_H

#include "math.h"
#include <vector>
#include <cstddef>

namespace treemaker {

	Vec3 getBezierPath(float t, Vec3 *points, size_t size);
	Vec3 getBezier(float t, Vec3 *points, size_t size);
}

#endif /* TM_CURVE_H */

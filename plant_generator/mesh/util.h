/* Copyright 2022 Floris Creyf
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

#ifndef PG_MESH_UTIL_H
#define PG_MESH_UTIL_H

#include "../plant.h"
#include "../stem.h"

inline float getAspect(const pg::Plant *plant, const pg::Stem *stem)
{
	float aspect = 1.0f;
	long materialID = stem->getMaterial(pg::Stem::Outer);
	if (materialID > 0) {
		pg::Material m = plant->getMaterial(materialID);
		aspect = m.getRatio();
	}
	return aspect;
}

#endif

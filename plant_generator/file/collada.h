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

#ifndef PG_COLLADA_H
#define PG_COLLADA_H

#include "../scene.h"
#include "../mesh/mesh.h"
#include <string>

namespace pg {
	class Collada {
		bool exportArmature = true;

	public:
		void exportFile(std::string filename, const Mesh &mesh,
			const Scene &scene);
	};
}

#endif

/* Copyright 2017-2018 Floris Creyf
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

#ifndef PG_STEM_POOL_H
#define PG_STEM_POOL_H

#include "stem.h"
#include <array>
#include <list>

#define PG_POOL_SIZE 100

namespace pg {
	class StemPool {
		struct Pool {
			long id;
			size_t remaining;
			std::array<Stem, PG_POOL_SIZE> stems;
		};
		std::list<Pool> pools;
		Stem *firstAvailable;
		long counter;

		Pool &addPool();
		std::list<Pool>::iterator getPool(Stem *stem);

	public:
		StemPool();
		StemPool(const StemPool &) = delete;
		Stem *allocate();
		size_t deallocate(Stem *stem);
		long getPoolID(const Stem *stem) const;
		size_t getRemaining(long id) const;
		size_t getPoolCount() const;
		size_t getPoolCapacity() const;
		void removePool(long id);
		void clear();
	};
}

#endif

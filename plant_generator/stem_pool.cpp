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

#include "stem_pool.h"
#include <cassert>

using namespace pg;
using std::list;
using std::array;

StemPool::StemPool() : firstAvailable(nullptr), counter(0)
{

}

Stem *StemPool::allocate()
{
	Stem *stem = this->firstAvailable;
	if (stem) {
		auto pool = getPool(stem);
		pool->remaining--;
	} else {
		Pool &pool = addPool();
		stem = this->firstAvailable;
		pool.remaining--;
	}
	this->firstAvailable = this->firstAvailable->nextAvailable;
	if (this->firstAvailable)
		this->firstAvailable->prevAvailable = nullptr;
	return stem;
}

StemPool::Pool &StemPool::addPool()
{
	assert(!this->firstAvailable);

	this->pools.push_back(Pool());
	Pool &pool = this->pools.back();
	pool.id = ++this->counter;
	pool.remaining = PG_POOL_SIZE;
	this->firstAvailable = &pool.stems[0];

	Stem *next = this->firstAvailable;
	Stem *prev = nullptr;
	for (int i = 0; i < PG_POOL_SIZE-1; i++) {
		pool.stems[i].prevAvailable = prev;
		prev = next;
		pool.stems[i].nextAvailable = ++next;
	}
	pool.stems[PG_POOL_SIZE-1].prevAvailable = prev;
	pool.stems[PG_POOL_SIZE-1].nextAvailable = nullptr;

	return pool;
}

size_t StemPool::deallocate(Stem *stem)
{
	list<Pool>::iterator it = getPool(stem);
	it->remaining++;
	if (this->firstAvailable) {
		stem->prevAvailable = nullptr;
		this->firstAvailable->prevAvailable = stem;
		stem->nextAvailable = this->firstAvailable;
		this->firstAvailable = stem;
	} else {
		this->firstAvailable = stem;
		stem->prevAvailable = nullptr;
		stem->nextAvailable = nullptr;
	}
	return it->remaining;
}

list<StemPool::Pool>::iterator StemPool::getPool(Stem *stem)
{
	for (auto it = this->pools.begin(); it != this->pools.end(); it++) {
		const void *start = &it->stems[0];
		const void *end = &it->stems[0] + PG_POOL_SIZE - 1;
		if (start <= stem && stem <= end)
			return it;
	}
	return this->pools.end();
}

long StemPool::getPoolID(const Stem *stem) const
{
	for (const auto &pool : this->pools) {
		const void *start = &pool.stems[0];
		const void *end = &pool.stems[0] + PG_POOL_SIZE - 1;
		if (start <= stem && stem <= end)
			return pool.id;
	}
	return 0;
}

size_t StemPool::getPoolCapacity() const
{
	return PG_POOL_SIZE;
}

size_t StemPool::getPoolCount() const
{
	return this->pools.size();
}

size_t StemPool::getRemaining(long id) const
{
	for (const auto &pool : this->pools)
		if (pool.id == id)
			return pool.remaining;
	return 0;
}

void StemPool::removePool(long id)
{
	for (auto it = this->pools.begin(); it != this->pools.end(); it++)
		if (it->id == id) {
			this->pools.erase(it);
			break;
		}
}

void StemPool::clear()
{
	this->pools.clear();
}

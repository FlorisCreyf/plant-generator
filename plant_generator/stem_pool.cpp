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

using namespace pg;
using std::list;
using std::array;

StemPool::StemPool()
{
	this->counter = 0;
}

Stem *StemPool::allocate()
{
	for (auto it = this->pools.begin(); it != this->pools.end(); it++) {
		Stem *stem = it->firstAvailable;
		if (stem && stem->unused) {
			stem->unused = false;
			it->firstAvailable = stem->nextAvailable;
			it->remaining--;
			return stem;
		}
	}
	Pool &pool = addPool();
	Stem *stem = pool.firstAvailable;
	stem->unused = false;
	pool.firstAvailable = stem->nextAvailable;
	pool.remaining--;
	return stem;
}

bool StemPool::allocateAt(Stem *stem)
{
	auto it = getPool(stem);
	if (it == this->pools.end())
		return false;
	stem->unused = false;
	it->remaining--;
	if (stem->nextAvailable)
		stem->nextAvailable->prevAvailable = stem->prevAvailable;
	if (stem->prevAvailable)
		stem->prevAvailable->nextAvailable = stem->nextAvailable;
	if (it->firstAvailable == stem)
		it->firstAvailable = stem->nextAvailable;
	return true;
}

StemPool::Pool &StemPool::addPool()
{
	this->pools.push_back(Pool());
	Pool &pool = this->pools.back();
	pool.id = ++this->counter;
	pool.remaining = PG_POOL_SIZE;
	pool.firstAvailable = &pool.stems[0];

	Stem *next = pool.firstAvailable;
	Stem *prev = nullptr;
	for (int i = 0; i < PG_POOL_SIZE-1; i++) {
		pool.stems[i].unused = true;
		pool.stems[i].prevAvailable = prev;
		prev = next;
		pool.stems[i].nextAvailable = ++next;
	}
	pool.stems[PG_POOL_SIZE-1].unused = true;
	pool.stems[PG_POOL_SIZE-1].prevAvailable = prev;
	pool.stems[PG_POOL_SIZE-1].nextAvailable = nullptr;

	return pool;
}

size_t StemPool::deallocate(Stem *stem)
{
	list<Pool>::iterator it = getPool(stem);
	stem->unused = true;
	it->remaining++;
	if (it->firstAvailable) {
		stem->prevAvailable = nullptr;
		it->firstAvailable->prevAvailable = stem;
		stem->nextAvailable = it->firstAvailable;
		it->firstAvailable = stem;
	} else {
		it->firstAvailable = stem;
		stem->prevAvailable = nullptr;
		stem->nextAvailable = nullptr;
	}
	return it->remaining;
}

list<StemPool::Pool>::iterator StemPool::getPool(Stem *stem)
{
	list<Pool>::iterator it = this->pools.begin();
	for (; it != this->pools.end(); it++) {
		const void *start = &it->stems[0];
		const void *end = &it->stems[0] + PG_POOL_SIZE - 1;
		if (start <= stem && stem <= end)
			return it;
	}
	return it;
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

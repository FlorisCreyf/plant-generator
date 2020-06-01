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

#include "joint.h"
#include "path.h"

using namespace pg;

Joint::Joint(unsigned id, unsigned parentID, size_t pathIndex)
{
	this->id = id;
	this->parentID = parentID;
	this->pathIndex = pathIndex;
}

unsigned Joint::getID() const
{
	return this->id;
}

unsigned Joint::getParentID() const
{
	return this->parentID;
}

size_t Joint::getPathIndex() const
{
	return this->pathIndex;
}

void Joint::updateLocation(Vec3 location)
{
	this->location = location;
}

Vec3 Joint::getLocation() const {
	return this->location;
}

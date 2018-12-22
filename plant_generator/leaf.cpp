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

#include "leaf.h"

pg::Leaf::Leaf()
{
	position = -1.0f;
	tilt = 0.0f;
	scale.x = 1.0f;
	scale.y = 2.0f;
}

bool pg::Leaf::operator==(const pg::Leaf &leaf) const
{
	return position == leaf.position;
}

bool pg::Leaf::operator!=(const pg::Leaf &leaf) const
{
	return !(*this == leaf);
}

void pg::Leaf::setPosition(float position)
{
	this->position = position;
}

float pg::Leaf::getPosition() const
{
	return position;
}

void pg::Leaf::setTilt(float tilt)
{
	this->tilt = tilt;
}

float pg::Leaf::getTilt() const
{
	return tilt;
}

void pg::Leaf::setScale(pg::Vec2 scale)
{
	this->scale = scale;
}

pg::Vec2 pg::Leaf::getScale() const
{
	return scale;
}

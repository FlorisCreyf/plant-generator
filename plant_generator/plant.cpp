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

#include "plant.h"

pg::Plant::Plant() : root(nullptr)
{

}

pg::Stem *pg::Plant::getRoot()
{
	return &root;
}

pg::Stem *pg::Plant::addStem(pg::Stem *stem)
{
	Stem *firstChild = stem->child;
	stem->child = new Stem(stem);
	if (firstChild)
		firstChild->prevSibling = stem->child;
	stem->child->nextSibling = firstChild;
	stem->child->prevSibling = nullptr;
	return stem->child;
}

void pg::Plant::removeStem(pg::Stem *stem)
{
	{
		Stem *child = stem->child;
		while (child) {
			Stem *next = child->nextSibling;
			removeStem(child);
			child = next;
		}
	}

	if (stem->prevSibling)
		stem->prevSibling->nextSibling = stem->nextSibling;
	if (stem->nextSibling)
		stem->nextSibling->prevSibling = stem->prevSibling;
	if (stem->parent && stem->parent->child == stem) {
		if (stem->prevSibling)
			stem->parent->child = stem->prevSibling;
		else
			stem->parent->child = stem->nextSibling;
	}

	delete stem;
}

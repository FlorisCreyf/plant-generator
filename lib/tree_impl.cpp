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

#include "tree_impl.h"
#include <limits>

using namespace treemaker;

treemaker::TreeImpl::TreeImpl() : root(nameGenerator, nullptr), 
		procGenerator(this), stemModifier(&procGenerator)
{

}

Stem *treemaker::TreeImpl::getRoot()
{
	return &root;
}

Stem *treemaker::TreeImpl::findStem(Stem *parent, unsigned name)
{
	if (name == parent->getName())
		return parent;

	for (size_t i = 0; i < parent->children.size(); i++) {
		Stem *stem = findStem(&parent->children[i], name);
		if (stem != nullptr)
			return stem;
	}

	return nullptr;
}

Stem *treemaker::TreeImpl::findStemByIndex(Stem *parent, size_t &index)
{
	if (index-- == 0)
		return parent;

	for (size_t i = 0; i < parent->children.size(); i++) {
		auto inf = std::numeric_limits<float>::infinity();
		if (parent->children[i].getLocation().x != inf) {
			Stem *s = findStemByIndex(&parent->children[i], index);
			if (s != nullptr)
				return s;
		}
	}

	return nullptr;
}

Stem *treemaker::TreeImpl::findStem(unsigned name)
{
	return findStem(&root, name);
}

Stem *treemaker::TreeImpl::findStemByIndex(size_t index)
{
	return findStemByIndex(&root, index);
}

void downgradeMode(Stem *stem)
{
	Stem *parent = stem->getParent();
	while (parent != nullptr) {
		parent->mode = MANUAL;
		parent = parent->getParent();
	}
}

void upgradeMode(Stem *stem)
{
	if (stem == nullptr)
		return;
	
	for (size_t i = 0; i < stem->getChildCount(); i++) {
		Stem *child = stem->getChild(i);
		child->mode = AUTOMATIC;
		upgradeMode(stem->getChild(i));
	}
}

void treemaker::TreeImpl::deleteStem(Stem *stem)
{
	if (stem->getParent() == nullptr)
		return;
	
	std::vector<Stem> &c = stem->getParent()->children;
	std::vector<Stem>::size_type i = (stem - &c[0]);
	
	if (stem->parent->hasDichotomous && i < 2) {
		stem->parent->hasDichotomous = false;
		c.erase(c.begin(), c.begin() + 2);
	} else
		c.erase(c.begin() + i);
}

void treemaker::TreeImpl::changeMode(Stem *stem, Flags mode)
{
	if (mode != AUTOMATIC) {
		if (mode == Flags::UNDEFINED && stem->mode == Flags::AUTOMATIC)
			mode = Flags::ASSISTED;
		
		switch (mode) {
		case Flags::MANUAL:
			stem->mode = Flags::MANUAL;
			downgradeMode(stem);
			break;
		case Flags::ASSISTED:
			stem->mode = Flags::ASSISTED;
			upgradeMode(stem);
			downgradeMode(stem);
			stemModifier.updateStemDensity(stem);
			break;
		default:
			break;
		}
	} else if (mode == AUTOMATIC) {
		stem->mode = AUTOMATIC;
		upgradeMode(stem);
		stemModifier.updateStemDensity(stem);
	}
}

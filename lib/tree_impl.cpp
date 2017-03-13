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

using namespace treemaker;

treemaker::TreeImpl::TreeImpl() : root(0, nullptr),  procGenerator(this)
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
		Stem *stem = findStemByIndex(&parent->children[i], index);
		if (stem != nullptr)
			return stem;
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

Stem *treemaker::TreeImpl::addStem(Stem *parent)
{
	Stem stem(name++, parent);
	parent->children.push_back(stem);
	return &parent->children.back();
}

void treemaker::TreeImpl::addDichotomousStems(Stem *parent)
{
	parent->hasDichotomous = true;
	Stem d1(name++, parent);
	Stem d2(name++, parent);
	float position = parent->path.getLength();
	d1.setPosition(position);
	d2.setPosition(position);
	parent->children.insert(parent->children.begin(), d2);
	parent->children.insert(parent->children.begin(), d1);
}

void treemaker::TreeImpl::removeDichotomousStems(Stem *parent)
{
	parent->hasDichotomous = false;
	auto start = parent->children.begin();
	auto end = parent->children.begin() + 2;
	parent->children.erase(start, end);
}

Stem *treemaker::TreeImpl::moveStem(Stem *stem, Stem *parent)
{
	return nullptr;
}

void treemaker::TreeImpl::removeStem(Stem *stem)
{

}

void treemaker::TreeImpl::removeLateralStems(Stem *parent)
{
	if (parent->hasDichotomousStems() && parent->children.size() >= 2) {
		auto begin = parent->children.begin() + 2;
		auto end = parent->children.end();
		parent->children.erase(begin, end);
	} else
		parent->children.clear();
}

/* Plant Generator
 * Copyright (C) 2020  Floris Creyf
 *
 * Plant Generator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Plant Generator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "generate.h"

using pg::Leaf;
using pg::Stem;
using pg::Plant;

Generate::Generate(Selection *selection, pg::PatternGenerator *generator) :
	selection(selection),
	prevSelection(*selection),
	removals(selection->getPlant()),
	remove(&removals),
	generator(generator)
{
	createRemovalSelection(this->selection, &this->removals);
	this->remove.execute();
	auto instances = this->selection->getStemInstances();
	for (auto &instance : instances) {
		Stem *stem = instance.first;
		this->parameterTrees.push_back(stem->getParameterTree());
		this->stems.push_back(*stem);
	}
}

void Generate::createRemovalSelection(Selection *selection, Selection *removals)
{
	selection->reduceToAncestors();
	selection->removeLeaves();
	auto instances = selection->getStemInstances();
	for (auto instance : instances) {
		Stem *stem = instance.first;
		size_t leafCount = stem->getLeafCount();
		for (size_t i = 0; i < leafCount; i++)
			if (!stem->getLeaf(i)->isCustom())
				removals->addLeaf(stem, i);

		Stem *child = stem->getChild();
		while (child) {
			if (!child->isCustom())
				removals->addStem(child);
			child = child->getSibling();
		}
	}
}

void Generate::removeAdditions()
{
	Plant *plant = this->selection->getPlant();
	auto instances = this->selection->getStemInstances();
	for (auto it = instances.rbegin(); it != instances.rend(); it++) {
		Stem *stem = it->first;
		for (int i = stem->getLeafCount() - 1; i >= 0; i--)
			if (!stem->getLeaf(i)->isCustom())
				stem->removeLeaf(i);

		Stem *child = stem->getChild();
		while (child) {
			Stem *sibling = child->getSibling();
			if (!child->isCustom())
				plant->deleteStem(child);
			child = sibling;
		}
	}
}

void Generate::execute()
{
	this->selection->reduceToAncestors();
	removeAdditions();
	auto instances = this->selection->getStemInstances();
	for (auto instance : instances)
		this->generator->grow(instance.first);
}

void Generate::undo()
{
	size_t index = 0;
	auto instances = this->selection->getStemInstances();
	this->parameterTree = instances.begin()->first->getParameterTree();
	for (auto instance : instances) {
		Stem *stem = instance.first;
		instance.first->setParameterTree(this->parameterTrees[index]);
		stem->setMinRadius(this->stems[index].getMinRadius());
		pg::Path path = this->stems[index].getPath();
		stem->setPath(path);
		index++;
	}
	removeAdditions();
	this->remove.undo();
	*this->selection = this->prevSelection;
}

void Generate::redo()
{
	createRemovalSelection(this->selection, &this->removals);
	this->remove.execute();
	auto instances = this->selection->getStemInstances();
	for (auto instance : instances)
		instance.first->setParameterTree(this->parameterTree);
	execute();
}

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

Generate::Generate(Selection *selection) :
	prevSelection(*selection),
	removals(selection->getPlant()),
	remove(&removals),
	gen(selection->getPlant())
{
	this->selection = selection;
	createRemovalSelection(selection, &this->removals);
	this->remove = RemoveStem(&this->removals);
	this->remove.execute();

	auto instances = selection->getStemInstances();
	for (auto &instance : instances)
		this->derivations.push_back(instance.first->getDerivation());
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

void Generate::setGenerator(pg::PseudoGenerator gen)
{
	this->gen = gen;
}

void Generate::removeAdditions()
{
	Selection additions(this->selection->getPlant());
	createRemovalSelection(this->selection, &additions);
	RemoveStem removeStem(&additions);
	removeStem.execute();
}

void Generate::execute()
{
	this->selection->reduceToAncestors();
	removeAdditions();
	for (auto instance : this->selection->getStemInstances()) {
		Stem *stem = instance.first;
		this->gen.grow(stem);
	}
}

void Generate::undo()
{
	size_t index = 0;
	auto instances = this->selection->getStemInstances();
	for (auto instance : instances)
		instance.first->setDerivation(this->derivations[index++]);

	createRemovalSelection(this->selection, &this->removals);
	RemoveStem removeGenerated(&this->removals);
	removeGenerated.execute();
	this->removals.clear();
	this->remove.undo();
	this->remove = removeGenerated;

	*this->selection = this->prevSelection;
}

void Generate::redo()
{
	createRemovalSelection(selection, &this->removals);
	RemoveStem removeOriginal(&this->removals);
	removeOriginal.execute();
	this->removals.clear();
	this->remove.undo();
	this->remove = removeOriginal;

	pg::DerivationTree dv = this->gen.getDerivation();
	auto instances = this->selection->getStemInstances();
	for (auto instance : instances)
		instance.first->setDerivation(dv);
}

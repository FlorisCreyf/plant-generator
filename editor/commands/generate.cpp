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

using pg::Stem;

Generate::Generate(Selection *selection) :
	prevSelection(*selection),
	removals(selection->getPlant()),
	additions(selection->getPlant()),
	removeRemovals(&removals),
	removeAdditions(&additions),
	gen(selection->getPlant())
{
	this->selection = selection;
}

void Generate::setGenerator(pg::PseudoGenerator gen)
{
	this->gen = gen;
}

void Generate::execute()
{
	this->selection->reduceToAncestors();
	Selection selection = *this->selection;
	this->removals = selection;
	this->removals.selectLeaves();
	this->removals.selectChildren();
	this->removeRemovals = RemoveStem(&this->removals);
	this->removeRemovals.execute();

	for (auto instance : selection.getStemInstances()) {
		Stem *stem = instance.first;
		this->gen.grow(stem);
	}

	this->additions = selection;
	this->additions.selectLeaves();
	this->additions.selectChildren();
	this->removeAdditions = RemoveStem(&this->additions);
}

void Generate::undo()
{
	this->removeAdditions.redo();
	this->removeRemovals.undo();
	*this->selection = this->prevSelection;
}

void Generate::redo()
{
	this->removeRemovals.redo();
	this->removeAdditions.undo();
	this->selection->reduceToAncestors();
}

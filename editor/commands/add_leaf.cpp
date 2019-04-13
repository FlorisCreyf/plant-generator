/* Plant Genererator
 * Copyright (C) 2019  Floris Creyf
 *
 * Plant Genererator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Plant Genererator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "add_leaf.h"

AddLeaf::AddLeaf(Selection *selection) : prevSelection(*selection)
{
	this->selection = selection;
	stem = nullptr;
	undone = false;
}

void AddLeaf::execute()
{
	auto instances = selection->getStemInstances();
	if (instances.size( ) == 1) {
		pg::Stem *stem = (*instances.begin()).first;
		pg::Leaf leaf;
		if (undone)
			leaf = this->leaf;
		else {
			this->leaf = leaf;
			this->stem = stem;
			prevSelection = *selection;
		}
		stem->addLeaf(leaf);
		selection->clear();
		selection->addLeaf(stem, leaf.getId());
	}
}

void AddLeaf::undo()
{
	if (stem) {
		printf("undo\n");
		stem->removeLeaf(leaf.getId());
		*selection = prevSelection;
		undone = true;
	}
}

AddLeaf *AddLeaf::clone()
{
	return new AddLeaf(*this);
}

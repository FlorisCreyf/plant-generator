/* Plant Genererator
 * Copyright (C) 2018  Floris Creyf
 *
 * Plant Genererator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.,
 *
 * Plant Genererator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "memorize_stem.h"

MemorizeStem::MemorizeStem(StemSelection *selection)
{
	this->selection = selection;
	undone = false;
}

bool MemorizeStem::isSameAsCurrent()
{
	if (selection) {
		auto instances = selection->getInstances();
		for (auto &instance : instances) {
			pg::Stem *stem = instance.first;
			auto it = stems.find(stem);
			if (it == stems.end() || it->second != *stem)
				return false;
		}
	}
	return true;
}

void MemorizeStem::execute()
{
	if (undone)
		swap();
	else {
		auto instances = selection->getInstances();
		for (auto &instance : instances) {
			stems.emplace(instance.first, *instance.first);
			break;
		}
	}
}

void MemorizeStem::undo()
{
	swap();
	undone = true;
}

void MemorizeStem::swap()
{
	auto instances = selection->getInstances();
	for (auto instance : instances) {
		pg::Stem *stem = instance.first;
		auto it = stems.find(stem);
		if (it != stems.end()) {
			pg::Stem temp = *stem;
			*stem = it->second;
			it->second = temp;
		}
	}
}

MemorizeStem *MemorizeStem::clone()
{
	return new MemorizeStem(*this);
}

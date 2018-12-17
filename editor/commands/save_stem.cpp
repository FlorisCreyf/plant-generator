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

#include "save_stem.h"

SaveStem::SaveStem(StemSelection *selection) :
	before(nullptr, nullptr), after(nullptr, nullptr)
{
	this->selection = selection;
	undone = false;
}

bool SaveStem::isSameAsCurrent()
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

void SaveStem::setNewSelection()
{
	after = *selection;
}

void SaveStem::execute()
{
	if (undone) {
		swap();
		*selection = after;
	} else {
		before = *selection;
		auto instances = selection->getInstances();
		for (auto &instance : instances) {
			stems.emplace(instance.first, *instance.first);
			break;
		}
	}
}

void SaveStem::undo()
{
	swap();
	*selection = before;
	undone = true;
}

void SaveStem::swap()
{
	auto instances = selection->getInstances();
	for (auto instance : instances) {
		pg::Stem *stem = instance.first;
		auto it = stems.find(stem);
		if (it != stems.end()) {
			pg::Stem s = *stem;
			*stem = it->second;
			it->second = s;
		}
	}
}

SaveStem *SaveStem::clone()
{
	return new SaveStem(*this);
}

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

#include "history.h"

History::History(pg::Plant *plant)
{
	this->plant = plant;
}

void History::add(pg::Stem *selectedStem, int selectedPoint)
{
	Memento m;
	m.stem = plant->copy(plant->getRoot(), nullptr, &selectedStem);
	m.selectedStem = selectedStem;
	m.selectedPoint = selectedPoint;
	past.push_back(m);

	for (size_t i = 0; i < future.size(); i++)
		plant->removeStem(future[i].stem);
	future.clear();
}

History::Memento History::undo(pg::Stem *selectedStem, int selectedPoint)
{
	Memento m;
	if (!past.empty()) {
		m.stem = plant->getRoot();
		m.selectedStem = selectedStem;
		m.selectedPoint = selectedPoint;
		future.push_back(m);

		m = past.back();
		plant->setRoot(m.stem);
		past.pop_back();
	}
	return m;
}

History::Memento History::redo(pg::Stem *selectedStem, int selectedPoint)
{
	Memento m;
	if (!future.empty()) {
		m.stem = plant->getRoot();
		m.selectedStem = selectedStem;
		m.selectedPoint = selectedPoint;
		past.push_back(m);

		m = future.back();
		plant->setRoot(m.stem);
		future.pop_back();
	}
	return m;
}

void History::clear()
{
	future.clear();
	past.clear();
}

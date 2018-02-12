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

#ifndef HISTORY_H
#define HISTORY_H

#include "plant_generator/plant.h"
#include <vector>

class History {
public:
	struct Memento {
		pg::Stem *stem = nullptr;
		pg::Stem *selectedStem = nullptr;
		int selectedPoint = -1;
	};

	History(pg::Plant *plant);
	void add(pg::Stem *selectedStem, int selectedPoint);
	Memento undo(pg::Stem *selectedStem, int selectedPoint);
	Memento redo(pg::Stem *selectedStem, int selectedPoint);

private:
	pg::Plant *plant;
	std::vector<Memento> past;
	std::vector<Memento> future;
};

#endif /* HISTORY_H */

/* Plant Genererator
 * Copyright (C) 2018  Floris Creyf
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

#ifndef POINT_SELECTION_STATE_H
#define POINT_SELECTION_STATE_H

#include "selection_state.h"
#include "editor/point_selection.h"
#include <memory>

class PointSelectionState : public SelectionState {
	PointSelection *selection;
	std::shared_ptr<PointSelection> copy;

public:
	PointSelectionState(PointSelection *selection);
	void replaceSelection();
	void swapSelection();
	PointSelectionState *clone() const;
	bool hasChanged() const;
};

#endif /* POINT_SELECTION_STATE_H */

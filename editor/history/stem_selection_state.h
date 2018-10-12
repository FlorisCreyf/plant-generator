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

#ifndef STEM_SELECTION_STATE_H
#define STEM_SELECTION_STATE_H

#include "selection_state.h"
#include "editor/stem_selection.h"

class StemSelectionState : public SelectionState {
	StemSelection *selection;
	std::shared_ptr<StemSelection> copy;
	
public:
	StemSelectionState(StemSelection *selection);
	void replaceSelection();
	void swapSelection();
	StemSelectionState *clone() const;
	bool hasChanged() const;
};

#endif /* STEM_SELECTION_STATE_H */


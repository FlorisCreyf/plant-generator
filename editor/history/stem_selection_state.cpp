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

#include "stem_selection_state.h"

StemSelectionState::StemSelectionState(StemSelection *selection) :
	copy(selection->clone())
{
	this->selection = selection;
}

void StemSelectionState::replaceSelection()
{
	*selection = *copy;
}

void StemSelectionState::swapSelection()
{
	*copy = *selection;
}

StemSelectionState *StemSelectionState::clone() const
{
	return new StemSelectionState(*this);
}

bool StemSelectionState::hasChanged() const
{
	return *selection != *copy;
}

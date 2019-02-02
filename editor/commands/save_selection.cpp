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

#include "save_selection.h"

SaveSelection::SaveSelection(Selection *selection) :
	after(*selection), before(*selection)
{
	this->selection = selection;
}

bool SaveSelection::hasChanged() const
{
	return *selection != before;
}

void SaveSelection::setBefore()
{
	before = *selection;
}

void SaveSelection::setAfter()
{
	after = *selection;
}

void SaveSelection::execute()
{
	*selection = after;
}

void SaveSelection::undo()
{
	*selection = before;
}

SaveSelection *SaveSelection::clone()
{
	return new SaveSelection(*this);
}

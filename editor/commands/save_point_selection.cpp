/* Plant Generator
 * Copyright (C) 2018  Floris Creyf
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

#include "save_point_selection.h"

SavePointSelection::SavePointSelection(PointSelection *selection) :
	after(*selection), before(*selection)
{
	this->selection = selection;
}

bool SavePointSelection::hasChanged() const
{
	return *selection != before;
}

void SavePointSelection::setBefore()
{
	before = *selection;
}

void SavePointSelection::setAfter()
{
	after = *selection;
}

void SavePointSelection::execute()
{
	*selection = after;
}

void SavePointSelection::undo()
{
	*selection = before;
}

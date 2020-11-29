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
	selection(selection), after(*selection), before(*selection)
{

}

bool SavePointSelection::hasChanged() const
{
	return *this->selection != this->before;
}

void SavePointSelection::setBefore()
{
	this->before = *this->selection;
}

void SavePointSelection::setAfter()
{
	this->after = *this->selection;
}

void SavePointSelection::execute()
{
	*this->selection = this->after;
}

void SavePointSelection::undo()
{
	*this->selection = this->before;
}

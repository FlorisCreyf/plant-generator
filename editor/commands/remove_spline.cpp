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

#include "remove_spline.h"

RemoveSpline::RemoveSpline(PointSelection *selection, pg::Spline *spline) :
	prevSelection(selection->clone())
{
	this->selection = selection;
	this->spline = spline;
	prevSpline = *spline;
	clearable = false;
}

void RemoveSpline::setClearable(bool clearable)
{
	this->clearable = clearable;
}

void RemoveSpline::execute()
{
	auto points = selection->getPoints();
	int size = spline->getControls().size();

	if (spline->getDegree() == 1) {
		if (size - points.size() < 2) {
			if (clearable) {
				spline->clear();
				points.clear();
			}
		} else {
			int i = 0;
			for (int index : points)
				spline->remove(index - i++);
			points.clear();
		}
	} else if (spline->getDegree() == 3) {
		std::set<int> list;
		for (int index : points) {
			if (index % 3 == 0)
				list.insert(index);
			else if (index % 3 == 1)
				list.insert(index - 1);
			else
				list.insert(index + 1);
		}

		if (size - (list.size() * 3) < 4) {
			if (clearable) {
				spline->clear();
				points.clear();
			}
		} else {
			std::set<int>::iterator it = list.begin();
			for (int i = 0; it != list.end(); ++it, ++i)
				spline->remove(*it - i * spline->getDegree());
			points.clear();
		}
	}

	selection->setPoints(points);
}

void RemoveSpline::undo()
{
	*selection = *prevSelection;
	*spline = prevSpline;
}

RemoveSpline *RemoveSpline::clone()
{
	return new RemoveSpline(*this);
}
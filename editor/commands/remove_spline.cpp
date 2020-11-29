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

#include "remove_spline.h"

RemoveSpline::RemoveSpline(PointSelection *selection, pg::Spline *spline) :
	selection(selection),
	prevSelection(*selection),
	spline(spline),
	prevSpline(*spline),
	clearable(false)
{

}

RemoveSpline::RemoveSpline(const RemoveSpline &original) :
	selection(original.selection),
	prevSelection(original.prevSelection),
	spline(original.spline),
	prevSpline(original.prevSpline),
	clearable(original.clearable)
{

}

RemoveSpline &RemoveSpline::operator=(const RemoveSpline &original)
{
	this->selection = original.selection;
	this->prevSelection = original.prevSelection;
	this->spline = original.spline;
	this->prevSpline = original.prevSpline;
	this->clearable = original.clearable;
	return *this;
}

void RemoveSpline::setClearable(bool clearable)
{
	this->clearable = clearable;
}

void RemoveSpline::execute()
{
	auto points = this->selection->getPoints();
	int size = this->spline->getControls().size();

	if (this->spline->getDegree() == 1) {
		if (size - points.size() < 2) {
			if (this->clearable) {
				this->spline->clear();
				points.clear();
			}
		} else {
			int i = 0;
			for (int index : points)
				this->spline->remove(index - i++);
			points.clear();
		}
	} else if (this->spline->getDegree() == 3) {
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
			if (this->clearable) {
				this->spline->clear();
				points.clear();
			}
		} else {
			std::set<int>::iterator it = list.begin();
			for (int i = 0; it != list.end(); ++it, ++i) {
				int degree = this->spline->getDegree();
				this->spline->remove(*it - i * degree);
			}
			points.clear();
		}
	}

	this->selection->setPoints(points);
}

void RemoveSpline::undo()
{
	*this->selection = this->prevSelection;
	*this->spline = this->prevSpline;
}

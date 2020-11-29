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

#include "extrude_spline.h"
#include "remove_spline.h"
#include <iterator>

ExtrudeSpline::ExtrudeSpline(
	PointSelection *selection, pg::Spline *spline, TranslationAxes *axes,
	const Camera *camera) :
	selection(selection),
	prevSelection(*selection),
	newSelection(*selection),
	spline(spline),
	moveSpline(selection, spline, axes, camera)
{

}

void ExtrudeSpline::setClickOffset(int x, int y)
{
	this->moveSpline.setClickOffset(x, y);
}

void ExtrudeSpline::execute()
{
	std::set<int> points = this->selection->getPoints();
	std::set<int> newPoints;
	int offset = 0;

	for (auto it = points.begin(); it != points.end(); ++it) {
		int point = *it;
		int i = point + offset;
		point = this->spline->insert(i, this->spline->getControls()[i]);
		newPoints.insert(point);
		offset += this->spline->getDegree();
	}

	this->selection->setPoints(newPoints);
}

void ExtrudeSpline::undo()
{
	this->newSelection = *this->selection;
	this->prevSpline = *this->spline;
	RemoveSpline remove(this->selection, this->spline);
	remove.execute();
	*this->selection = this->prevSelection;
}

void ExtrudeSpline::redo() {
	*this->spline = this->prevSpline;
	*this->selection = this->newSelection;
}

bool ExtrudeSpline::onMouseMove(QMouseEvent *event)
{
	bool update = this->moveSpline.onMouseMove(event);
	this->done = this->moveSpline.isDone();
	return update;
}

bool ExtrudeSpline::onMousePress(QMouseEvent *event)
{
	bool update = this->moveSpline.onMousePress(event);
	this->done = this->moveSpline.isDone();
	return update;
}

bool ExtrudeSpline::onMouseRelease(QMouseEvent *event)
{
	bool update = this->moveSpline.onMouseRelease(event);
	this->done = this->moveSpline.isDone();
	return update;
}

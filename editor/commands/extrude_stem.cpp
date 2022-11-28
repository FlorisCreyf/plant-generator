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

#include "extrude_stem.h"
#include "remove_stem.h"

ExtrudeStem::ExtrudeStem(Selection *selection, TranslationAxes *axes,
	const Camera *camera) :
	selection(selection),
	prevSelection(*selection),
	newSelection(*selection),
	axes(axes),
	camera(camera),
	movePath(selection, axes, camera)
{

}

bool ExtrudeStem::isValid(const Selection &selection)
{
	return selection.hasPoints();
}

void ExtrudeStem::setClickOffset(int x, int y)
{
	pg::Vec3 average = this->selection->getAveragePosition();
	pg::Vec3 s = this->camera->toScreenSpace(average);
	this->movePath.setClickOffset(s.x - x, s.y - y);
	this->axes->selectCenter();
}

void ExtrudeStem::execute()
{
	auto instances = this->selection->getStemInstances();
	for (auto &instance : instances) {
		pg::Stem *stem = instance.first;
		std::set<int> points = instance.second.getPoints();
		std::set<int> newPoints;

		pg::Path path = stem->getPath();
		pg::Spline spline = path.getSpline();
		int offset = 0;
		for (int point : points) {
			int i = point + offset;
			point = spline.insert(i, spline.getControls()[i]);
			newPoints.insert(point);
			offset += spline.getDegree();
		}

		path.setSpline(spline);
		stem->setPath(path);
		instance.second.setPoints(newPoints);
	}
	this->selection->setInstances(instances);
}

void ExtrudeStem::undo()
{
	this->newSelection = *this->selection;
	this->prevSplines.clear();
	auto instances = this->selection->getStemInstances();
	for (auto &instance : instances) {
		pg::Stem *stem = instance.first;
		pg::Path path = stem->getPath();
		pg::Spline spline = path.getSpline();
		this->prevSplines.emplace(stem, spline);
	}

	RemoveStem remove(this->selection);
	remove.execute();
	*this->selection = this->prevSelection;
}

void ExtrudeStem::redo()
{
	for (auto item : this->prevSplines) {
		pg::Stem *stem = item.first;
		auto path = stem->getPath();
		path.setSpline(item.second);
		stem->setPath(path);
	}
	*this->selection = this->newSelection;
}

bool ExtrudeStem::onMouseMove(QMouseEvent *event)
{
	bool update = this->movePath.onMouseMove(event);
	this->done = this->movePath.isDone();
	return update;
}

bool ExtrudeStem::onMousePress(QMouseEvent *event)
{
	bool update = this->movePath.onMousePress(event);
	this->done = this->movePath.isDone();
	return update;
}

bool ExtrudeStem::onMouseRelease(QMouseEvent *event)
{
	bool update = this->movePath.onMouseRelease(event);
	this->done = this->movePath.isDone();
	return update;
}

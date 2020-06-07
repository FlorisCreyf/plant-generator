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

ExtrudeStem::ExtrudeStem(
	Selection *selection, TranslationAxes *axes, Camera *camera) :
	prevSelection(*selection),
	newSelection(*selection),
	movePath(selection, axes, camera)
{
	this->selection = selection;
	this->axes = axes;
	this->camera = camera;
}

void ExtrudeStem::setClickOffset(int x, int y)
{
	pg::Vec3 average = selection->getAveragePosition();
	pg::Vec3 s = camera->toScreenSpace(average);
	movePath.setClickOffset(s.x - x, s.y - y);
	axes->selectCenter();
}

void ExtrudeStem::execute()
{
	auto instances = selection->getStemInstances();
	for (auto &instance : instances) {
		pg::Stem *stem = instance.first;
		std::set<int> points = instance.second.getPoints();
		std::set<int> newPoints;

		pg::Path path = stem->getPath();
		pg::Spline spline = path.getSpline();
		int o = 0;
		for (int point : points) {
			int l = point + o;
			point = spline.insert(l, spline.getControls()[l]);
			newPoints.insert(point);
			o += spline.getDegree();
		}

		path.setSpline(spline);
		stem->setPath(path);
		instance.second.setPoints(newPoints);
	}
	selection->setInstances(instances);
}

void ExtrudeStem::undo()
{
	newSelection = *selection;
	prevSplines.clear();
	auto instances = selection->getStemInstances();
	for (auto &instance : instances) {
		pg::Stem *stem = instance.first;
		pg::Path path = stem->getPath();
		pg::Spline spline = path.getSpline();
		prevSplines.emplace(stem, spline);
	}

	RemoveStem remove(selection);
	remove.execute();
	*selection = prevSelection;
}

void ExtrudeStem::redo()
{
	for (auto item : prevSplines) {
		pg::Stem *stem = item.first;
		auto path = stem->getPath();
		path.setSpline(item.second);
		stem->setPath(path);
	}
	*selection = newSelection;
}

bool ExtrudeStem::onMouseMove(QMouseEvent *event)
{
	bool update = movePath.onMouseMove(event);
	done = movePath.isDone();
	return update;
}

bool ExtrudeStem::onMousePress(QMouseEvent *event)
{
	bool update = movePath.onMousePress(event);
	done = movePath.isDone();
	return update;
}

bool ExtrudeStem::onMouseRelease(QMouseEvent *event)
{
	bool update = movePath.onMouseRelease(event);
	done = movePath.isDone();
	return update;
}

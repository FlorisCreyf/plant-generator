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

#include "extrude_stem.h"
#include "remove_stem.h"

ExtrudeStem::ExtrudeStem(StemSelection *selection) :
	prevSelection(*selection), newSelection(*selection)
{
	this->selection = selection;
}

void ExtrudeStem::extrude()
{
	auto instances = selection->getInstances();
	for (auto &instance : instances) {
		pg::Stem *stem = instance.first;
		std::set<int> points = instance.second.getPoints();
		std::set<int> newPoints;

		pg::VolumetricPath path = stem->getPath();
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

void ExtrudeStem::execute()
{
	if (prevSplines.empty())
		extrude();
	else
		redo();
}

void ExtrudeStem::undo()
{
	/* Memorize state before undoing. */
	newSelection = *selection;
	prevSplines.clear();
	auto instances = selection->getInstances();
	for (auto &instance : instances) {
		pg::Stem *stem = instance.first;
		pg::VolumetricPath path = stem->getPath();
		pg::Spline spline = path.getSpline();
		prevSplines.emplace(stem, spline);
	}

	RemoveStem remove(selection);
	remove.execute();
	*selection = prevSelection;
}

ExtrudeStem *ExtrudeStem::clone()
{
	return new ExtrudeStem(*this);
}

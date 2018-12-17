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

#include "move_path.h"

MovePath::MovePath(StemSelection *selection, TranslationAxes *axes) :
	moveSpline(nullptr, nullptr, axes)
{
	this->selection = selection;
	this->axes = axes;
	undoing = false;
}

void MovePath::set(Camera &camera, int x, int y)
{
	moveSpline.set(camera.getRay(x, y), camera.getDirection());
}

void MovePath::setParallelTangents(bool parallel)
{
	moveSpline.setParallelTangents(parallel);
}

void MovePath::execute()
{
	auto instances = selection->getInstances();
	for (auto &instance : instances) {
		pg::Stem *stem = instance.first;
		PointSelection &ps = instance.second;
		auto points = ps.getPoints();

		if (points.empty())
			continue;

		pg::VolumetricPath path = stem->getPath();
		pg::Spline spline = path.getSpline();
		moveSpline.setSelection(&instance.second);
		moveSpline.setSpline(&spline);
		if (undoing)
			moveSpline.undo();
		else
			moveSpline.execute();
		path.setSpline(spline);
		stem->setPath(path);
	}
}

void MovePath::undo()
{
	undoing = true;
	execute();
	undoing = false;
}

MovePath *MovePath::clone()
{
	return new MovePath(*this);
}

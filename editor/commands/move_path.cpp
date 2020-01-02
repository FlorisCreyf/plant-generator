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

MovePath::MovePath(
	const Selection *selection, TranslationAxes *axes,
	const Camera *camera) : moveSpline(nullptr, nullptr, axes, camera)
{
	this->camera = camera;
	this->selection = selection;
	this->axes = axes;
	this->clickOffset[0] = this->clickOffset[1] = 0.0f;
	this->undoing = false;
}

void MovePath::setClickOffset(int x, int y)
{
	this->clickOffset[0] = x;
	this->clickOffset[1] = y;
}

void MovePath::setParallelTangents(bool parallel)
{
	this->moveSpline.setParallelTangents(parallel);
}

void MovePath::execute()
{
	auto instances = this->selection->getStemInstances();
	for (auto &instance : instances) {
		pg::Stem *stem = instance.first;
		PointSelection &ps = instance.second;
		auto points = ps.getPoints();

		if (points.empty())
			continue;

		pg::Path path = stem->getPath();
		pg::Spline spline = path.getSpline();
		this->moveSpline.setSelection(&instance.second);
		this->moveSpline.setSpline(&spline);
		if (this->undoing)
			this->moveSpline.undo();
		else
			this->moveSpline.execute();
		path.setSpline(spline);
		stem->setPath(path);
	}
}

void MovePath::undo()
{
	this->undoing = true;
	execute();
}

void MovePath::redo()
{
	this->undoing = false;
	execute();
}

bool MovePath::onMouseMove(QMouseEvent *event)
{
	QPoint point = event->pos();
	bool ctrl = event->modifiers() & Qt::ControlModifier;
	float x = point.x() + clickOffset[0];
	float y = point.y() + clickOffset[1];
	setParallelTangents(!ctrl);
	this->moveSpline.set(camera->getRay(x, y), camera->getDirection());
	execute();
	return true;
}

bool MovePath::onMousePress(QMouseEvent *)
{
	return false;
}

bool MovePath::onMouseRelease(QMouseEvent *)
{
	this->done = true;
	return false;
}

bool MovePath::onKeyPress(QKeyEvent *)
{
	return false;
}

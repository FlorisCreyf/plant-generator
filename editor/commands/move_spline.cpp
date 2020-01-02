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

#include "move_spline.h"

using pg::Vec3;

MoveSpline::MoveSpline(
	const PointSelection *selection, pg::Spline *spline,
	TranslationAxes *axes, const Camera *camera)
{
	this->parallel = true;
	this->clickOffset[0] = this->clickOffset[1] = 0.0f;
	this->totalDirection = pg::getZeroVec3();
	this->selection = selection;
	this->spline = spline;
	this->axes = axes;
	this->camera = camera;
}

void MoveSpline::preservePositions()
{
	this->positions = this->spline->getControls();
}

void MoveSpline::setClickOffset(int x, int y)
{
	this->clickOffset[0] = x;
	this->clickOffset[1] = y;
}

void MoveSpline::setSelection(const PointSelection *selection)
{
	this->selection = selection;
}

void MoveSpline::setSpline(pg::Spline *spline)
{
	this->spline = spline;
}

void MoveSpline::set(pg::Ray ray, Vec3 cameraDirection)
{
	float distance;
	Vec3 position = axes->getPosition();
	pg::Plane plane = {position, cameraDirection};

	if (axes->getSelection() == Axes::XAxis)
		plane.normal.x = 0.0f;
	else if (axes->getSelection() == Axes::YAxis)
		plane.normal.y = 0.0f;
	else if (axes->getSelection() == Axes::ZAxis)
		plane.normal.z = 0.0f;

	plane.normal = pg::normalize(plane.normal);
	distance = pg::intersectsPlane(ray, plane);
	position = distance * ray.direction + ray.origin;

	if (axes->getSelection() == Axes::XAxis) {
		position.y = plane.point.y;
		position.z = plane.point.z;
	} else if (axes->getSelection() == Axes::YAxis) {
		position.x = plane.point.x;
		position.z = plane.point.z;
	} else if (axes->getSelection() == Axes::ZAxis) {
		position.x = plane.point.x;
		position.y = plane.point.y;
	}

	this->direction = position - axes->getPosition();
	this->totalDirection += direction;
	axes->setPosition(position);
}

void MoveSpline::setParallelTangents(bool parallel)
{
	this->parallel = parallel;
}

Vec3 MoveSpline::getDirection()
{
	return direction;
}

void MoveSpline::execute()
{
	auto points = selection->getPoints();

	/* The new location of each point is the difference of the
	position of the axes before and after the movement. */
	if (spline->getDegree() == 1) {
		for (int point : points) {
			Vec3 location = spline->getControls()[point];
			spline->move(point, location + direction, true);
		}
	} else if (spline->getDegree() == 3) {
		for (auto it = points.begin(); it != points.end(); ++it) {
			int p = *it;
			bool a = it == points.begin();
			bool b = it == --points.end();

			/* Don't move peripheral points if the center
			point is also selected. */
			if (p % 3 == 2 && (!b && *std::next(it) == p + 1))
				continue;
			if (p % 3 == 1 && (!a && *std::prev(it) == p - 1))
				continue;

			/* Only one point can be moved if one peripheral
			point moves the other. */
			Vec3 location = spline->getControls()[p];
			if (p % 3 == 1 && (!a && *std::prev(it) == p - 2))
				spline->move(p, location + direction, false);
			else if (p % 3 == 2 && (!b && *std::next(it) == p + 2))
				spline->move(p, location + direction, false);
			else
				spline->move(p, location + direction, parallel);
		}
	}
}

void MoveSpline::undo()
{
	if (this->positions.empty()) {
		this->totalDirection = -1.0f * this->totalDirection;
		this->direction = this->totalDirection;
		execute();
	} else {
		std::vector<Vec3> controls = this->spline->getControls();
		this->spline->setControls(this->positions);
		this->positions = controls;
	}
}

void MoveSpline::redo()
{
	undo();
}

bool MoveSpline::onMouseMove(QMouseEvent *event)
{
	QPoint point = event->pos();
	float x = point.x() + clickOffset[0];
	float y = point.y() + clickOffset[1];
	set(this->camera->getRay(x, y), this->camera->getDirection());
	setParallelTangents(false);
	execute();
	return true;
}

bool MoveSpline::onMousePress(QMouseEvent *)
{
	return false;
}

bool MoveSpline::onMouseRelease(QMouseEvent *)
{
	this->done = true;
	return false;
}

bool MoveSpline::onKeyPress(QKeyEvent *)
{
	return false;
}

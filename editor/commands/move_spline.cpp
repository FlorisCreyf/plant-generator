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

#include "move_spline.h"

using pg::Vec3;

MoveSpline::MoveSpline(
	const PointSelection *selection, pg::Spline *spline,
	TranslationAxes *axes, const Camera *camera) :
	selection(selection),
	camera(camera),
	axes(axes),
	spline(spline),
	totalDirection(0.0f, 0.0f, 0.0f),
	parallel(true),
	clickOffset{0, 0}
{

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
	Vec3 position = this->axes->getPosition();
	pg::Plane plane = {position, cameraDirection};

	if (this->axes->getSelection() == Axes::XAxis)
		plane.normal.x = 0.0f;
	else if (this->axes->getSelection() == Axes::YAxis)
		plane.normal.y = 0.0f;
	else if (this->axes->getSelection() == Axes::ZAxis)
		plane.normal.z = 0.0f;

	plane.normal = pg::normalize(plane.normal);
	distance = pg::intersectsPlane(ray, plane);
	position = distance * ray.direction + ray.origin;

	if (this->axes->getSelection() == Axes::XAxis) {
		position.y = plane.point.y;
		position.z = plane.point.z;
	} else if (this->axes->getSelection() == Axes::YAxis) {
		position.x = plane.point.x;
		position.z = plane.point.z;
	} else if (this->axes->getSelection() == Axes::ZAxis) {
		position.x = plane.point.x;
		position.y = plane.point.y;
	}

	this->direction = position - this->axes->getPosition();
	this->totalDirection += this->direction;
	this->axes->setPosition(position);
}

void MoveSpline::setParallelTangents(bool parallel)
{
	this->parallel = parallel;
}

Vec3 MoveSpline::getDirection()
{
	return this->direction;
}

void MoveSpline::execute()
{
	auto points = this->selection->getPoints();

	/* The new location of each point is the difference of the
	position of the axes before and after the movement. */
	if (this->spline->getDegree() == 1) {
		for (int point : points) {
			Vec3 location = this->spline->getControls()[point];
			location += this->direction;
			this->spline->move(point, location, true);
		}
	} else if (this->spline->getDegree() == 3) {
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
			Vec3 location = this->spline->getControls()[p];
			location += this->direction;
			if (p % 3 == 1 && (!a && *std::prev(it) == p - 2))
				this->spline->move(p, location, false);
			else if (p % 3 == 2 && (!b && *std::next(it) == p + 2))
				this->spline->move(p, location, false);
			else
				this->spline->move(p, location, this->parallel);
		}
	}
}

void MoveSpline::undo()
{
	if (this->positions.empty()) {
		this->totalDirection = -1.0f * this->totalDirection;
		this->direction = this->totalDirection;
		execute();
		this->direction = -1.0f * this->direction;
		this->totalDirection = this->direction;
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
	float x = point.x() + this->clickOffset[0];
	float y = point.y() + this->clickOffset[1];
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

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

MoveSpline::MoveSpline(PointSelection *selection, pg::Spline *spline,
	TranslationAxes *axes)
{
	parallel = true;
	totalDirection = pg::getZeroVec3();
	this->selection = selection;
	this->spline = spline;
	this->axes = axes;
}

void MoveSpline::setSelection(PointSelection *selection)
{
	this->selection = selection;
}

void MoveSpline::setSpline(pg::Spline *spline)
{
	this->spline = spline;
}

void MoveSpline::set(pg::Ray ray, pg::Vec3 cameraDirection)
{
	float distance;
	pg::Vec3 position = axes->getPosition();
	pg::Plane plane = {position, cameraDirection};

	switch (axes->getSelection()) {
	case Axes::XAxis:
		plane.normal.x = 0.0f;
		break;
	case Axes::YAxis:
		plane.normal.y = 0.0f;
		break;
	case Axes::ZAxis:
		plane.normal.z = 0.0f;
		break;
	default:
		break;
	}

	plane.normal = pg::normalize(plane.normal);
	distance = pg::intersectsPlane(ray, plane);
	position = distance * ray.direction + ray.origin;

	switch (axes->getSelection()) {
	case Axes::XAxis:
		position.y = plane.point.y;
		position.z = plane.point.z;
		break;
	case Axes::YAxis:
		position.x = plane.point.x;
		position.z = plane.point.z;
		break;
	case Axes::ZAxis:
		position.x = plane.point.x;
		position.y = plane.point.y;
		break;
	default:
		break;
	}

	direction = position - axes->getPosition();
	totalDirection += direction;
	axes->setPosition(position);
}

void MoveSpline::setParallelTangents(bool parallel)
{
	this->parallel = parallel;
}

pg::Vec3 MoveSpline::getDirection()
{
	return direction;
}

void MoveSpline::execute()
{
	auto points = selection->getPoints();

	/* The new location of each point is the difference of the
	 * position of the axes before and after the movement. */
	if (spline->getDegree() == 1) {
		for (int point : points) {
			pg::Vec3 location = spline->getControls()[point];
			spline->move(point, location + direction, true);
		}
	} else if (spline->getDegree() == 3) {
		for (auto it = points.begin(); it != points.end(); ++it) {
			int p = *it;
			bool a = it == points.begin();
			bool b = it == --points.end();
			
			/* Don't move peripheral points if the center
			 * point is also selected. */
			if (p % 3 == 2 && (!b && *std::next(it) == p + 1))
				continue;
			if (p % 3 == 1 && (!a && *std::prev(it) == p - 1))
				continue;
			
			/* Only one point can be moved if one peripheral
			 * point moves the other. */
			pg::Vec3 location = spline->getControls()[p];
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
	direction = totalDirection = -1.0f * totalDirection;
	execute();
	/* Update direction for redo. */
	direction = totalDirection = -1.0f * totalDirection;
}

MoveSpline *MoveSpline::clone()
{
	return new MoveSpline(*this);
}

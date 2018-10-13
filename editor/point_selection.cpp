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

#include "point_selection.h"
#include <cmath>

using pg::Vec3;
using pg::Spline;

PointSelection::PointSelection(Camera *camera)
{
	this->camera = camera;
	location.x = location.y = location.z = 0.0f;
}

bool PointSelection::operator==(const PointSelection &obj) const
{
	return location == obj.location && points == obj.points;
}

bool PointSelection::operator!=(const PointSelection &obj) const
{
	return !(*this == obj);
}

PointSelection *PointSelection::clone()
{
	return new PointSelection(*this);
}

int PointSelection::selectPoint(QMouseEvent *event, const Spline &spline,
	pg::Vec3 location)
{
	bool ctrl = event->modifiers() & Qt::ControlModifier;
	auto controls = spline.getControls();
	int size = controls.size();
	int degree = spline.getDegree();
	int selectedPoint = -1;

	/* Find what point is clicked on. */
	for (int i = 0; i < size; i++) {
		Vec3 point = controls[i] + location;
		point = camera->toScreenSpace(point);
		float sx = std::pow(point.x - event->pos().x(), 2);
		float sy = std::pow(point.y - event->pos().y(), 2);

		if (degree == 3 && i % 3 == 0) {
			if (std::sqrt(sx + sy) < 5) {
				selectedPoint = i;
				break;
			}
		} else if (std::sqrt(sx + sy) < 10) {
			if (degree == 3 && selectedPoint % 3 != 0)
				selectedPoint = i;
			else {
				selectedPoint = i;
				break;
			}
		}
	}

	/* Remove previous selections if no modifier key is pressed. */
	if (!ctrl)
		points.clear();

	if (selectedPoint >= 0) {
		/* Remove the point from the selection if it is already
		 * selected. */
		std::set<int>::iterator it = points.find(selectedPoint);
		if (it != points.end())
			points.erase(it);
		else
			points.insert(selectedPoint);
	} else if (!ctrl)
		/* Remove the entire selection if nothing was clicked on. */
		points.clear();

	return selectedPoint;
}

void PointSelection::setPoints(std::set<int> points)
{
	this->points = points;
}

std::set<int> PointSelection::getPoints() const
{
	return points;
}

bool PointSelection::hasPoints() const
{
	return !points.empty();
}

bool PointSelection::contains(int point) const
{
	return points.find(point) != points.end();
}

Vec3 PointSelection::getAveragePosition(const Spline &spline, pg::Vec3 location)
	const
{
	Vec3 position = {0.0f, 0.0f, 0.0f};
	for (int point : points)
		position += spline.getControls()[point];
	position /= points.size();
	position += location;
	return position;
}

void PointSelection::clear()
{
	points.clear();
}

void PointSelection::selectNext(int max)
{
	if (!points.empty()) {
		for (int i = *points.begin() + 1; i < max; i++)
			points.insert(i);
	}
}

void PointSelection::selectPrevious()
{
	if (!points.empty()) {
		for (int i = *points.begin() - 1; i >= 0; i--)
			points.insert(i);
	}
}

void PointSelection::selectAll(int max)
{
	for (int i = 0; i < max; i++)
		points.insert(i);
}

void PointSelection::select(int point)
{
	points.insert(point);
}

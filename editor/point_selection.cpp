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

#include "point_selection.h"
#include <cmath>

using pg::Vec3;
using pg::Spline;

bool PointSelection::operator==(const PointSelection &obj) const
{
	return this->points == obj.points;
}

bool PointSelection::operator!=(const PointSelection &obj) const
{
	return !(*this == obj);
}

void PointSelection::setPoints(std::set<int> points)
{
	this->points = points;
}

void PointSelection::addPoint(int point)
{
	this->points.insert(point);
}

std::set<int> PointSelection::getPoints() const
{
	return this->points;
}

bool PointSelection::hasPoints() const
{
	return !this->points.empty();
}

bool PointSelection::contains(int point) const
{
	return this->points.find(point) != this->points.end();
}

Vec3 PointSelection::getAveragePosition(const Spline &spline, Vec3 location)
	const
{
	Vec3 position(0.0f, 0.0f, 0.0f);
	for (int point : this->points)
		position += spline.getControls()[point];
	position /= this->points.size();
	position += location;
	return position;
}

void PointSelection::clear()
{
	this->points.clear();
}

bool PointSelection::removePoint(int point)
{
	return this->points.erase(point) > 0;
}

void PointSelection::selectNext(int max)
{
	if (!this->points.empty())
		for (int i = *this->points.begin() + 1; i < max; i++)
			this->points.insert(i);
}

void PointSelection::selectPrevious()
{
	if (!this->points.empty())
		for (int i = *this->points.begin() - 1; i >= 0; i--)
			this->points.insert(i);
}

void PointSelection::selectAll(int max)
{
	for (int i = 0; i < max; i++)
		this->points.insert(i);
}

void PointSelection::select(int point)
{
	this->points.insert(point);
}

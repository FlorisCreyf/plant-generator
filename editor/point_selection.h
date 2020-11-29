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

#ifndef POINT_SELECTION_H
#define POINT_SELECTION_H

#include "plant_generator/spline.h"
#include <set>

class PointSelection {
	std::set<int> points;

public:
	PointSelection();
	bool operator==(const PointSelection &obj) const;
	bool operator!=(const PointSelection &obj) const;

	void setPoints(std::set<int> points);
	void addPoint(int point);
	std::set<int> getPoints() const;
	bool hasPoints() const;
	bool contains(int point) const;

	pg::Vec3 getAveragePosition(
		const pg::Spline &spline, pg::Vec3 location) const;

	void clear();
	bool removePoint(int point);
	void selectNext(int max);
	void selectPrevious();
	void selectAll(int max);
	void select(int point);
};

#endif

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

#ifndef POINT_SELECTION_H
#define POINT_SELECTION_H

#include "camera.h"
#include "plant_generator/spline.h"
#include <QtGui/QMouseEvent>
#include <set>

class PointSelection {
	Camera *camera;
	std::set<int> points;
	pg::Vec3 location;

public:
	PointSelection(Camera *camera);
	bool operator==(const PointSelection &obj) const;
	bool operator!=(const PointSelection &obj) const;

	/** Returns the index of the point that was clicked on. */
	int selectPoint(QMouseEvent *event, const pg::Spline &spline,
		pg::Vec3 location);
	void setPoints(std::set<int> points);
	std::set<int> getPoints() const;
	bool hasPoints() const;
	bool contains(int point) const;

	pg::Vec3 getAveragePosition(const pg::Spline &spline,
		pg::Vec3 location) const;

	void clear();
	void selectNext(int max);
	void selectPrevious();
	void selectAll(int max);
	void select(int point);
};

#endif /* POINT_SELECTION_H */

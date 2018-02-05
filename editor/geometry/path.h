/* Plant Genererator
 * Copyright (C) 2018  Floris Creyf
 *
 * TreeMaker is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TreeMaker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PATH_H
#define PATH_H

#include "plant_generator/math/math.h"
#include "plant_generator/spline.h"
#include "geometry.h"
#include <vector>

class Path {
	Geometry path;
	Geometry::Segment lineSegment;
	Geometry::Segment pointSegment;
	pg::Vec3 curveColor;
	pg::Vec3 controlColor;
	unsigned restartIndex;

public:
	void setColor(pg::Vec3 curveColor, pg::Vec3 controlColor);
	void set(const pg::Spline &spline, int resolution);
	void set(const std::vector<pg::Vec3> &points,
		const std::vector<pg::Vec3> &controls, int degree);
	const Geometry *getGeometry();
	Geometry::Segment getLineSegment();
	Geometry::Segment getPointSegment();
};

#endif /* PATH_H */

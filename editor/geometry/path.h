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

#ifndef PATH_H
#define PATH_H

#include "editor/point_selection.h"
#include "plant_generator/math/math.h"
#include "plant_generator/spline.h"
#include "geometry.h"
#include <vector>
#include <set>

class Path {
public:
	struct Segment {
		pg::Spline spline;
		int resolution;
		pg::Vec3 location;
	};

	void setColor(pg::Vec3 curve, pg::Vec3 control, pg::Vec3 selection);
	void set(const pg::Spline &spline, int resolution, pg::Vec3 location);
	void set(std::vector<Segment> segments);
	void setSelectedPoints(const PointSelection &selection, int index = 0);
	const Geometry *getGeometry();
	Geometry::Segment getLineSegment();
	Geometry::Segment getPointSegment();
	void clearPoints();

private:
	Geometry path;
	Geometry::Segment lineSegment;
	Geometry::Segment pointSegment;
	pg::Vec3 curveColor;
	pg::Vec3 controlColor;
	pg::Vec3 selectionColor;
	std::vector<pg::Vec3> points;
	std::vector<unsigned> indices;
	std::vector<int> resolution;
	std::vector<int> degree;
	std::vector<int> controlStart;
	std::vector<int> pointStart;
	std::vector<int> lineStart;

	int setPoints(const std::vector<Segment> &segments, int index);
	void setControls(const std::vector<Segment> &segments, int index);
	std::vector<pg::Vec3> getControls(Segment &segment);

	void set(const std::vector<pg::Vec3> &points,
		const std::vector<pg::Vec3> &controls, int degree);

	void colorLinear(const std::set<int> &points, int index);
	void colorBezier(const std::set<int> &points, int index);
	void colorLine(int point, int index);
};

#endif /* PATH_H */

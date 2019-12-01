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

#include "path.h"
#include "editor/point_selection.h"
#include <iterator>

using pg::Vec3;
using std::vector;

void Path::setColor(Vec3 curve, Vec3 control, Vec3 selection)
{
	this->curveColor = curve;
	this->controlColor = control;
	this->selectionColor = selection;
}

void Path::set(const pg::Spline &spline, int resolution, pg::Vec3 location)
{
	if (!spline.getControls().empty()) {
		Segment segment;
		segment.spline = spline;
		segment.resolution = resolution;
		segment.location = location;
		std::vector<Segment> segments;
		segments.push_back(segment);
		set(segments);
	}
}

void Path::set(std::vector<Segment> segments)
{
	int index = 0;
	path.clear();
	controlStart.clear();
	pointStart.clear();
	lineStart.clear();
	resolution.clear();
	degree.clear();

	for (auto &segment : segments) {
		this->resolution.push_back(segment.resolution);
		this->degree.push_back(segment.spline.getDegree());
	}

	index = setPoints(segments, index);
	setControls(segments, index);
}

int Path::setPoints(const std::vector<Path::Segment> &segments, int index)
{
	for (Segment s : segments) {
		lineStart.push_back(index);

		std::vector<Vec3> points;
		float step = 1.0f / s.resolution;
		int curves = s.spline.getCurveCount();
		for (int curve = 0; curve < curves; curve++) {
			float t = 0.0f;
			for (int i = 0; i < s.resolution; i++, t += step) {
				Vec3 p = s.spline.getPoint(curve, t);
				points.push_back(s.location + p);

			}
		}
		Vec3 lastPoint = s.spline.getPoint(curves - 1, 1.0f);
		points.push_back(s.location + lastPoint);

		/* curve */
		int size = index + points.size();
		for (int i = 0; index < size; i++) {
			path.addPoint(points[i], curveColor);
			path.addIndex(index++);
		}

		path.addIndex(Geometry::primitiveReset);
	}
	return index;
}

void Path::setControls(const std::vector<Path::Segment> &segments, int index)
{
	int curve = 0;
	for (Segment s : segments) {
		if (degree[curve++] == 3) {
			controlStart.push_back(index);

			std::vector<Vec3> controls = getControls(s);

			/* control lines */
			size_t i = 0;

			path.addPoint(controls[i++], controlColor);
			path.addIndex(index++);
			path.addPoint(controls[i++], controlColor);
			path.addIndex(index++);
			path.addIndex(Geometry::primitiveReset);

			while (i < controls.size() - 2) {
				path.addPoint(controls[i++], controlColor);
				path.addIndex(index++);
				path.addPoint(controls[i++], controlColor);
				path.addIndex(index++);
				path.addPoint(controls[i++], controlColor);
				path.addIndex(index++);
				path.addIndex(Geometry::primitiveReset);
			}

			path.addPoint(controls[i++], controlColor);
			path.addIndex(index++);
			path.addPoint(controls[i++], controlColor);
			path.addIndex(index++);

			path.addIndex(Geometry::primitiveReset);
		}
	}
	lineSegment = path.getSegment();

	int size = 0;
	for (Segment s : segments) {
		pointStart.push_back(index);

		std::vector<Vec3> controls = getControls(s);

		/* control points */
		for (size_t i = 0; i < controls.size(); i++) {
			path.addPoint(controls[i], controlColor);
			index++;
		}
		size += controls.size();
	}
	pointSegment.pstart = lineSegment.pcount;
	pointSegment.pcount = size;
}

std::vector<pg::Vec3> Path::getControls(Segment& segment)
{
	std::vector<Vec3> controls = segment.spline.getControls();
	for (auto it = controls.begin(); it != controls.end(); ++it)
		*it += segment.location;
	return controls;
}

void Path::setSelectedPoints(PointSelection &selection, int index)
{
	/* Assumes the color of a point determines the color of the entire
	 * line. */
	auto points = selection.getPoints();
	if (!points.empty()) {
		if (degree[index] == 1)
			colorLinear(points, index);
		else if (degree[index] == 3)
			colorBezier(points, index);
	}
}

void Path::colorLinear(const std::set<int> &points, int index)
{
	std::set<int>::iterator it;

	for (it = points.begin(); it != points.end(); ++it) {
		int offset = pointStart[index] + *it;
		path.changeColor(offset, selectionColor);
		if (std::next(it) != points.end() && *std::next(it) == *it + 1)
			colorLine(*it, index);
	}
}

void Path::colorBezier(const std::set<int> &points, int index)
{
	std::set<int>::iterator it;
	for (it = points.begin(); it != points.end(); ++it) {
		int p = *it;

		/* Change color of points. */
		int offset = pointStart[index] + p;
		path.changeColor(offset, selectionColor);

		/* Change color of lines between selected points. */
		auto nx = std::next(it);
		auto pv = std::prev(it);
		if (p % 3 == 0 && points.find(p + 3) != points.end()) {
			colorLine(p/3, index);
		} else if (p % 3 == 1 && it != points.begin() && *pv == p - 1) {
			int i = (controlStart[index] + p);
			path.changeColor(i, selectionColor);
		} else if (p % 3 == 2 && nx != points.end() && *nx == p + 1) {
			int i = (1 + controlStart[index] + p);
			path.changeColor(i, selectionColor);
		}
	}
}

void Path::colorLine(int point, int index)
{
	int offset = lineStart[index] + resolution[index] * point;
	for (int i = 1; i <= resolution[index]; i++)
		path.changeColor(offset + i, selectionColor);
}

const Geometry *Path::getGeometry()
{
	return &path;
}

Geometry::Segment Path::getLineSegment()
{
	return lineSegment;
}

Geometry::Segment Path::getPointSegment()
{
	return pointSegment;
}

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

void Path::set(const pg::Spline &spline, int resolution, Vec3 location)
{
	if (!spline.getControls().empty()) {
		Segment segment;
		segment.spline = spline;
		segment.resolution = resolution;
		segment.location = location;
		vector<Segment> segments;
		segments.push_back(segment);
		set(segments);
	}
}

void Path::set(vector<Segment> segments)
{
	this->path.clear();
	this->controlStart.clear();
	this->pointStart.clear();
	this->lineStart.clear();
	this->resolution.clear();
	this->degree.clear();

	for (auto &segment : segments) {
		this->resolution.push_back(segment.resolution);
		this->degree.push_back(segment.spline.getDegree());
	}

	int index = setPoints(segments, 0);
	setControls(segments, index);
}

int Path::setPoints(const vector<Path::Segment> &segments, int index)
{
	for (Segment segment : segments) {
		vector<Vec3> points;
		this->lineStart.push_back(index);

		float step = 1.0f / segment.resolution;
		int curves = segment.spline.getCurveCount();
		for (int curve = 0; curve < curves; curve++) {
			float t = 0.0f;
			for (int i = 0; i < segment.resolution; i++) {
				Vec3 point = segment.spline.getPoint(curve, t);
				points.push_back(segment.location + point);
				t += step;
			}
		}
		Vec3 lastPoint = segment.spline.getPoint(curves - 1, 1.0f);
		points.push_back(segment.location + lastPoint);

		int size = index + points.size();
		for (int i = 0; index < size; i++) {
			this->path.addPoint(points[i], curveColor);
			this->path.addIndex(index++);
		}

		this->path.addIndex(Geometry::primitiveReset);
	}
	return index;
}

void Path::setControls(const vector<Path::Segment> &segments, int index)
{
	int curve = 0;
	for (Segment segment : segments) {
		if (this->degree[curve++] == 3) {
			vector<Vec3> controls = getControls(segment);
			size_t i = 0;

			this->controlStart.push_back(index);

			this->path.addPoint(controls[i++], this->controlColor);
			this->path.addIndex(index++);
			this->path.addPoint(controls[i++], this->controlColor);
			this->path.addIndex(index++);
			this->path.addIndex(Geometry::primitiveReset);

			while (i < controls.size() - 2) {
				this->path.addPoint(
					controls[i++], this->controlColor);
				this->path.addIndex(index++);
				this->path.addPoint(
					controls[i++], this->controlColor);
				this->path.addIndex(index++);
				this->path.addPoint(
					controls[i++], this->controlColor);
				this->path.addIndex(index++);
				this->path.addIndex(Geometry::primitiveReset);
			}

			this->path.addPoint(controls[i++], this->controlColor);
			this->path.addIndex(index++);
			this->path.addPoint(controls[i++], this->controlColor);
			this->path.addIndex(index++);

			this->path.addIndex(Geometry::primitiveReset);
		}
	}
	this->lineSegment = this->path.getSegment();

	int size = 0;
	for (Segment segment : segments) {
		this->pointStart.push_back(index);
		vector<Vec3> controls = getControls(segment);
		for (size_t i = 0; i < controls.size(); i++, index++)
			this->path.addPoint(controls[i], this->controlColor);
		size += controls.size();
	}
	this->pointSegment.pstart = this->lineSegment.pcount;
	this->pointSegment.pcount = size;
}

vector<Vec3> Path::getControls(Segment& segment)
{
	vector<Vec3> controls = segment.spline.getControls();
	for (auto it = controls.begin(); it != controls.end(); ++it)
		*it += segment.location;
	return controls;
}

/** Assumes the color of a point determines the color of the entire line. */
void Path::setSelectedPoints(PointSelection &selection, int index)
{
	auto points = selection.getPoints();
	if (!points.empty()) {
		if (this->degree[index] == 1)
			colorLinear(points, index);
		else if (degree[index] == 3)
			colorBezier(points, index);
	}
}

void Path::colorLinear(const std::set<int> &points, int index)
{
	std::set<int>::iterator it;
	for (it = points.begin(); it != points.end(); ++it) {
		int offset = this->pointStart[index] + *it;
		this->path.changeColor(offset, this->selectionColor);
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
		int offset = this->pointStart[index] + p;
		this->path.changeColor(offset, this->selectionColor);

		/* Change color of lines between selected points. */
		auto nx = std::next(it);
		auto pv = std::prev(it);
		if (p % 3 == 0 && points.find(p + 3) != points.end()) {
			colorLine(p/3, index);
		} else if (p % 3 == 1 && it != points.begin() && *pv == p - 1) {
			int i = (this->controlStart[index] + p);
			this->path.changeColor(i, this->selectionColor);
		} else if (p % 3 == 2 && nx != points.end() && *nx == p + 1) {
			int i = (1 + this->controlStart[index] + p);
			this->path.changeColor(i, this->selectionColor);
		}
	}
}

void Path::colorLine(int point, int index)
{
	int offset = this->lineStart[index] + this->resolution[index] * point;
	for (int i = 1; i <= this->resolution[index]; i++)
		this->path.changeColor(offset + i, this->selectionColor);
}

const Geometry *Path::getGeometry()
{
	return &this->path;
}

Geometry::Segment Path::getLineSegment()
{
	return this->lineSegment;
}

Geometry::Segment Path::getPointSegment()
{
	return this->pointSegment;
}

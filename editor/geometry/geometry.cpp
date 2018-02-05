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

#include "geometry.h"
#include <cmath>

using pg::Vec3;
using pg::Mat4;

void Geometry::clear()
{
	points.clear();
	indices.clear();
}

void Geometry::addIndex(unsigned index)
{
	indices.push_back(index);
}

void Geometry::addPoint(Vec3 point, Vec3 color)
{
	points.push_back(point.x);
	points.push_back(point.y);
	points.push_back(point.z);
	points.push_back(color.x);
	points.push_back(color.y);
	points.push_back(color.z);
}

void Geometry::addLine(Vec3 line[2], Vec3 color)
{
	addPoint(line[0], color);
	addPoint(line[1], color);
}

void Geometry::addCurve(const pg::Spline &spline, pg::Vec3 color, int size)
{
	float step = 1.0f/(size - 1);
	for (int curve = 0; curve < spline.getCurveCount(); curve++) {
		float t = 0.0f;
		for (int i = 0; i < size; i++, t += step)
			addPoint(spline.getPoint(curve, t), color);
	}
}

void Geometry::addPlane(Vec3 a, Vec3 b, Vec3 c, Vec3 color)
{
	addPoint(c, color);
	addPoint(c + a, color);
	addPoint(c + b, color);
	addPoint(c + a + b, color);

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(2);
	indices.push_back(1);
	indices.push_back(3);
}

void Geometry::addCone(float radius, float height, int points, Vec3 color)
{
	unsigned pointStart = this->points.size()/6;

	for (int i = 0; i < points; i++) {
		float r = i * 2.0f * M_PI / points;
		Vec3 point = {std::cos(r)*radius, 0.0f, std::sin(r)*radius};
		addPoint(point, color);

		indices.push_back(pointStart + i);
		indices.push_back(pointStart + points);
		indices.push_back(pointStart + (i == points-1 ? 0 : i+1));

		indices.push_back(pointStart + i);
		indices.push_back(pointStart + points + 1);
		indices.push_back(pointStart + (i == points-1 ? 0 : i+1));
	}

	addPoint({0.0f, height, 0.0f}, color);
	addPoint({0.0f, 0.0f, 0.0f}, color);
}

void Geometry::addGrid(int size, pg::Vec3 pcolor, pg::Vec3 scolor)
{
	Vec3 line[2];
	float bound = static_cast<float>(size);

	line[0] = {0.0f, 0.0f, bound};
	line[1] = {0.0f, 0.0f, -bound};
	addLine(line, pcolor);
	line[0] = {bound, 0.0f, 0.0f};
	line[1] = {-bound, 0.0f, 0.0f};
	addLine(line, pcolor);

	for (int i = 1; i <= size; i++) {
		float j = static_cast<float>(i);

		line[0] = {j, 0.0f, bound};
		line[1] = {j, 0.0f, -bound};
		addLine(line, scolor);
		line[0] = {-j, 0.0f, bound};
		line[1] = {-j, 0.0f, -bound};
		addLine(line, scolor);
		line[0] = {bound, 0.0f, j};
		line[1] = {-bound, 0.0f, j};
		addLine(line, scolor);
		line[0] = {bound, 0.0f, -j};
		line[1] = {-bound, 0.0f, -j};
		addLine(line, scolor);
	}
}

void Geometry::transform(size_t start, size_t count, const Mat4 &transform)
{
	const int vertexSize = 6;
	const int length = (start + count) * vertexSize;
	for (int i = start * vertexSize; i < length; i += vertexSize) {
		Vec3 v = {points[i], points[i+1], points[i+2]};
		v = pg::toVec3(transform * pg::toVec4(v, 1.0f));
		points[i+0] = v.x;
		points[i+1] = v.y;
		points[i+2] = v.z;
	}
}

Geometry::Segment Geometry::getSegment() const
{
	Segment s;
	s.pstart = 0;
	s.pcount = points.size()/6;
	s.istart = 0;
	s.icount = indices.size();
	return s;
}

Geometry::Segment Geometry::append(const Geometry &geometry)
{
	auto ps = geometry.points;
	auto is = geometry.indices;
	Segment s = geometry.getSegment();
	s.pstart = points.size()/6;
	s.pcount = ps.size()/6;
	s.istart = indices.size();
	s.icount = is.size();
	for (size_t i = 0; i < is.size(); i++)
		if (is[i] != primitiveReset)
			is[i] += s.pstart;
	points.insert(points.end(), ps.begin(), ps.end());
	indices.insert(indices.end(), is.begin(), is.end());
	return s;
}


const std::vector<float> *Geometry::getPoints() const
{
	return &points;
}

const std::vector<unsigned> *Geometry::getIndices() const
{
	return &indices;
}

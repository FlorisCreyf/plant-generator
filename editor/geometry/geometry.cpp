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

#include "geometry.h"
#include <cmath>

const float pi = 3.14159265359f;

using pg::DVertex;
using pg::Volume;
using pg::Vec2;
using pg::Vec3;
using pg::Mat4;
using std::cos;
using std::sin;
using std::vector;

Geometry::Geometry() : points(), indices()
{

}

void Geometry::clear()
{
	points.clear();
	indices.clear();
}

void Geometry::addIndex(unsigned index)
{
	indices.push_back(index);
}

void Geometry::addPoint(Vec3 point, Vec3 normal)
{
	DVertex vertex;
	vertex.position = point;
	vertex.normal = normal;
	this->points.push_back(vertex);
}

void Geometry::addLine(Vec3 line[2], Vec3 color)
{
	DVertex vertex;
	vertex.normal = color;
	vertex.position = line[0];
	this->points.push_back(vertex);
	vertex.position = line[1];
	this->points.push_back(vertex);
}

void Geometry::addCircle(float radius, int points, Vec3 color)
{
	float angle = 0.0f;
	float rotation = 2.0f * pi / points;
	for (int i = 0; i < points; i++) {
		Vec3 point(cos(angle)*radius, sin(angle)*radius, 0.0f);
		addPoint(point, color);
		angle += rotation;
	}
}

void Geometry::addCurve(const pg::Spline &spline, Vec3 color, int size)
{
	float step = 1.0f/(size - 1);
	for (int curve = 0; curve < spline.getCurveCount(); curve++) {
		float t = 0.0f;
		for (int i = 0; i < size; i++, t += step)
			addPoint(spline.getPoint(curve, t), color);
	}
}

void Geometry::addPlane(Vec3 a, Vec3 b, Vec3 c, Vec3 normal, Vec3 tangent)
{
	DVertex vertex;
	vertex.normal = normal;
	vertex.tangent = tangent;
	vertex.tangentScale = 1.0f;

	vertex.position = c;
	vertex.uv = Vec2(0.0f, 0.0f);
	this->points.push_back(vertex);
	vertex.position = c + a;
	vertex.uv = Vec2(1.0f, 0.0f);
	this->points.push_back(vertex);
	vertex.position = c + b;
	vertex.uv = Vec2(0.0f, 1.0f);
	this->points.push_back(vertex);
	vertex.position = c + a + b;
	vertex.uv = Vec2(1.0f, 1.0f);
	this->points.push_back(vertex);

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(2);
	indices.push_back(1);
	indices.push_back(3);
}

void Geometry::addCone(float radius, float height, int points, Vec3 color)
{
	unsigned pointStart = this->points.size();

	for (int i = 0; i < points; i++) {
		float r = i * 2.0f * pi / points;
		addPoint(Vec3(cos(r)*radius, sin(r)*radius, 0.0f), color);

		indices.push_back(pointStart + i);
		indices.push_back(pointStart + points);
		indices.push_back(pointStart + (i == points-1 ? 0 : i+1));

		indices.push_back(pointStart + i);
		indices.push_back(pointStart + points + 1);
		indices.push_back(pointStart + (i == points-1 ? 0 : i+1));
	}

	addPoint(Vec3(0.0f, 0.0f, height), color);
	addPoint(Vec3(0.0f, 0.0f, 0.0f), color);
}

void Geometry::addGrid(int size, Vec3 pcolor[2], Vec3 scolor)
{
	Vec3 line[2];
	float bound = static_cast<float>(size);

	line[0] = Vec3(0.0f, bound, 0.0f);
	line[1] = Vec3(0.0f, -bound, 0.0f);
	addLine(line, pcolor[1]);
	line[0] = Vec3(bound, 0.0f, 0.0f);
	line[1] = Vec3(-bound, 0.0f, 0.0f);
	addLine(line, pcolor[0]);

	for (int i = 1; i <= size; i++) {
		float j = static_cast<float>(i);
		line[0] = Vec3(j, bound, 0.0f);
		line[1] = Vec3(j, -bound, 0.0f);
		addLine(line, scolor);
		line[0] = Vec3(-j, bound, 0.0f);
		line[1] = Vec3(-j, -bound, 0.0f);
		addLine(line, scolor);
		line[0] = Vec3(bound, j, 0.0f);
		line[1] = Vec3(-bound, j, 0.0f);
		addLine(line, scolor);
		line[0] = Vec3(bound, -j, 0.0f);
		line[1] = Vec3(-bound, -j, 0.0f);
		addLine(line, scolor);
	}
}

void Geometry::addCube(Vec3 center, float size, Vec3 color)
{
	Vec3 line[2];
	/* bottom square */
	line[0] = center + Vec3(size, -size, -size);
	line[1] = center + Vec3(-size, -size, -size);
	addLine(line, color);
	line[0] = center + Vec3(size, size, -size);
	line[1] = center + Vec3(-size, size, -size);
	addLine(line, color);
	line[0] = center + Vec3(size, size, -size);
	line[1] = center + Vec3(size, -size, -size);
	addLine(line, color);
	line[0] = center + Vec3(-size, size, -size);
	line[1] = center + Vec3(-size, -size, -size);
	addLine(line, color);
	/* top square */
	line[0] = center + Vec3(size, -size, size);
	line[1] = center + Vec3(-size, -size, size);
	addLine(line, color);
	line[0] = center + Vec3(size, size, size);
	line[1] = center + Vec3(-size, size, size);
	addLine(line, color);
	line[0] = center + Vec3(size, size, size);
	line[1] = center + Vec3(size, -size, size);
	addLine(line, color);
	line[0] = center + Vec3(-size, size, size);
	line[1] = center + Vec3(-size, -size, size);
	addLine(line, color);
	/* vertical lines */
	line[0] = center + Vec3(size, -size, size);
	line[1] = center + Vec3(size, -size, -size);
	addLine(line, color);
	line[0] = center + Vec3(size, size, size);
	line[1] = center + Vec3(size, size, -size);
	addLine(line, color);
	line[0] = center + Vec3(-size, size, size);
	line[1] = center + Vec3(-size, size, -size);
	addLine(line, color);
	line[0] = center + Vec3(-size, -size, size);
	line[1] = center + Vec3(-size, -size, -size);
	addLine(line, color);
}

Vec3 getVolumeColor(const Volume::Node *node)
{
	float a = node->getDensity();
	float c = a*0.9f+0.1f;
	return Vec3(0.1f, c, c);
}

void Geometry::addVolume(const Volume *volume)
{
	const Volume::Node *node = volume->getRoot();
	Vec3 center = node->getCenter();
	float size = node->getSize();
	addCube(center, size, getVolumeColor(node));
	divideVolume(node);
}

void Geometry::divideVolume(const Volume::Node *parent)
{
	Vec3 center = parent->getCenter();
	float size = parent->getSize();
	Vec3 color = getVolumeColor(parent);
	if (parent->getNode(0)) {
		Vec3 line[2];
		/* vertical lines */
		line[0] = center + Vec3(0.0f, 0.0f, -size);
		line[1] = center + Vec3(0.0f, 0.0f, size);
		addLine(line, color);
		line[0] = center + Vec3(0.0f, size, -size);
		line[1] = center + Vec3(0.0f, size, size);
		addLine(line, color);
		line[0] = center + Vec3(0.0f, -size, -size);
		line[1] = center + Vec3(0.0f, -size, size);
		addLine(line, color);
		line[0] = center + Vec3(size, 0.0f, -size);
		line[1] = center + Vec3(size, 0.0f, size);
		addLine(line, color);
		line[0] = center + Vec3(-size, 0.0f, -size);
		line[1] = center + Vec3(-size, 0.0f, size);
		addLine(line, color);
		/* outer horizontal lines */
		line[0] = center + Vec3(size, -size, 0.0f);
		line[1] = center + Vec3(size, size, 0.0f);
		addLine(line, color);
		line[0] = center + Vec3(-size, -size, 0.0f);
		line[1] = center + Vec3(-size, size, 0.0f);
		addLine(line, color);
		line[0] = center + Vec3(-size, size, 0.0f);
		line[1] = center + Vec3(size, size, 0.0f);
		addLine(line, color);
		line[0] = center + Vec3(-size, -size, 0.0f);
		line[1] = center + Vec3(size, -size, 0.0f);
		addLine(line, color);
		/* inner horizontal lines */
		line[0] = center + Vec3(0.0f, -size, -size);
		line[1] = center + Vec3(0.0f, size, -size);
		addLine(line, color);
		line[0] = center + Vec3(-size, 0.0f, -size);
		line[1] = center + Vec3(size, 0.0f, -size);
		addLine(line, color);
		line[0] = center + Vec3(0.0f, -size, 0.0f);
		line[1] = center + Vec3(0.0f, size, 0.0f);
		addLine(line, color);
		line[0] = center + Vec3(-size, 0.0f, 0.0f);
		line[1] = center + Vec3(size, 0.0f, 0.0f);
		addLine(line, color);
		line[0] = center + Vec3(0.0f, -size, size);
		line[1] = center + Vec3(0.0f, size, size);
		addLine(line, color);
		line[0] = center + Vec3(-size, 0.0f, size);
		line[1] = center + Vec3(size, 0.0f, size);
		addLine(line, color);
		for (int i = 0; i < 8; i++)
			divideVolume(parent->getNode(i));
	}
}

void Geometry::transform(size_t start, size_t count, const Mat4 &transform)
{
	const int length = start + count;
	for (int i = start; i < length; i++)
		points[i].position = transform.apply(points[i].position, 1.0f);
}

void Geometry::changeColor(size_t index, Vec3 color)
{
	points[index].normal = color;
}

Geometry::Segment Geometry::getSegment() const
{
	Segment segment;
	segment.pstart = 0;
	segment.pcount = points.size();
	segment.istart = 0;
	segment.icount = indices.size();
	return segment;
}

Geometry::Segment Geometry::append(const Geometry &geometry)
{
	auto ps = geometry.points;
	auto is = geometry.indices;
	Segment segment;
	segment.pstart = points.size();
	segment.pcount = ps.size();
	segment.istart = indices.size();
	segment.icount = is.size();
	for (size_t i = 0; i < is.size(); i++)
		if (is[i] != primitiveReset)
			is[i] += segment.pstart;
	this->points.insert(this->points.end(), ps.begin(), ps.end());
	this->indices.insert(this->indices.end(), is.begin(), is.end());
	return segment;
}

const vector<DVertex> *Geometry::getPoints() const
{
	return &points;
}

const vector<unsigned> *Geometry::getIndices() const
{
	return &indices;
}

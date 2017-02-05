/* TreeMaker: 3D tree model editor
 * Copyright (C) 2017  Floris Creyf
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
#include "curve.h"

using std::vector;
using namespace graphics;

void insertVec3(vector<float>::iterator &itr, TMvec3 a)
{
	*(itr++) = a.x;
	*(itr++) = a.y;
	*(itr++) = a.z;
}

void insertPoint(vector<float>::iterator &itr, TMvec3 a, TMvec3 color)
{
	insertVec3(itr, a);
	insertVec3(itr, color);
}

void insertLine(vector<float>::iterator &itr, TMvec3 a, TMvec3 b, TMvec3 color)
{
	insertVec3(itr, a);
	insertVec3(itr, color);
	insertVec3(itr, b);
	insertVec3(itr, color);
}

VertexFormat Geometry::getVertexFormat()
{
	return VERTEX_COLOR;
}

Fragment Geometry::newFragment(int vsize, int isize, GLenum type)
{
	Fragment fragment;
	int formatSize = graphics::getSize(VERTEX_COLOR);
	fragment.start[0] = (vertices.size() - vsize) / formatSize;
	fragment.start[1] = indices.size() - isize;
	fragment.count[0] = vsize / formatSize;
	fragment.count[1] = isize;
	fragment.type = type;
	return fragment;
}

Fragment Geometry::addGrid(int sections, TMvec3 primColor, TMvec3 secColor)
{
	const int SIZE = (8*sections+4)*6;
	float bound = static_cast<float>(sections);
	vector<float>::iterator itr;

	vertices.resize(vertices.size() + SIZE);
	itr = vertices.end() - SIZE;

	insertPoint(itr, {0.0f, 0.0f, bound}, primColor);
	insertPoint(itr, {0.0f, 0.0f, -bound}, primColor);
	insertPoint(itr, {bound, 0.0f, 0.0f}, primColor);
	insertPoint(itr, {-bound, 0.0f, 0.0f}, primColor);

	for (int i = 1; i <= sections; i++) {
		float j = static_cast<float>(i);

		insertPoint(itr, {j, 0.0f, bound}, secColor);
		insertPoint(itr, {j, 0.0f, -bound}, secColor);
		insertPoint(itr, {-j, 0.0f, bound}, secColor);
		insertPoint(itr, {-j, 0.0f, -bound}, secColor);

		insertPoint(itr, {bound, 0.0f, j}, secColor);
		insertPoint(itr, {-bound, 0.0f, j}, secColor);
		insertPoint(itr, {bound, 0.0f, -j}, secColor);
		insertPoint(itr, {-bound, 0.0f, -j}, secColor);
	}

	return newFragment(SIZE, 0, GL_LINES);
}

Fragment Geometry::addBox(TMaabb &b, TMvec3 color)
{
	const int SIZE = 144;
	vector<float>::iterator itr;

	vertices.resize(vertices.size() + SIZE);
	itr = vertices.end() - SIZE;

	insertLine(itr, {b.x1, b.y1, b.z1}, {b.x2, b.y1, b.z1}, color);
	insertLine(itr, {b.x1, b.y1, b.z1}, {b.x1, b.y2, b.z1}, color);
	insertLine(itr, {b.x1, b.y1, b.z1}, {b.x1, b.y1, b.z2}, color);
	insertLine(itr, {b.x2, b.y2, b.z2}, {b.x1, b.y2, b.z2}, color);
	insertLine(itr, {b.x2, b.y2, b.z2}, {b.x2, b.y1, b.z2}, color);
	insertLine(itr, {b.x2, b.y2, b.z2}, {b.x2, b.y2, b.z1}, color);
	insertLine(itr, {b.x2, b.y1, b.z1}, {b.x2, b.y2, b.z1}, color);
	insertLine(itr, {b.x2, b.y1, b.z1}, {b.x2, b.y1, b.z2}, color);
	insertLine(itr, {b.x1, b.y2, b.z1}, {b.x2, b.y2, b.z1}, color);
	insertLine(itr, {b.x1, b.y2, b.z1}, {b.x1, b.y2, b.z2}, color);
	insertLine(itr, {b.x1, b.y1, b.z2}, {b.x2, b.y1, b.z2}, color);
	insertLine(itr, {b.x1, b.y1, b.z2}, {b.x1, b.y2, b.z2}, color);

	return newFragment(SIZE, 0, GL_LINES);
}

Fragment Geometry::addLine(vector<TMvec3> points, TMvec3 color)
{
	const int SIZE = points.size() * 6;
	vector<float>::iterator itr;

	vertices.resize(vertices.size() + SIZE);
	itr = vertices.end() - SIZE;

	for (int i = points.size() - 1; i >= 0; --i) {
		insertVec3(itr, points[i]);
		insertVec3(itr, color);
	}

	return newFragment(SIZE, 0, GL_LINE_STRIP);
}

Fragment Geometry::addBezier(vector<TMvec3> points, int divisions, TMvec3 color)
{
	const int SIZE = divisions * 6;
	vector<float>::iterator itr;

	vertices.resize(vertices.size() + SIZE);
	itr = vertices.end() - SIZE;

	for (int i = 0; i < divisions; i++) {
		float t = i/(float)(divisions-1);
		insertVec3(itr, tmGetBezier(t, &points[0], points.size()));
		insertVec3(itr, color);
	}

	return newFragment(SIZE, 0, GL_LINE_STRIP);
}

Fragment Geometry::addBPath(vector<TMvec3> points, int divisions, TMvec3 color)
{
	const int CURVES = points.size() / 4;
	const int SIZE = divisions * 6 * CURVES;
	vector<float>::iterator itr;

	vertices.resize(vertices.size() + SIZE);
	itr = vertices.end() - SIZE;

	for (int i = 0; i < CURVES; i++)
		for (int j = 0; j < divisions; j++) {
			float t = j/(float)(divisions-1);
			insertVec3(itr, tmGetBezier(t, &points[i*4], 4));
			insertVec3(itr, color);
		}

	return newFragment(SIZE, 0, GL_LINE_STRIP);
}

Fragment Geometry::addPlane(TMvec3 a, TMvec3 b, TMvec3 c, TMvec3 color)
{
	const int SIZE = 36;
	const int VERTEX_START = vertices.size()/6;
	vector<float>::iterator itr;

	vertices.resize(vertices.size() + SIZE);
	itr = vertices.end() - SIZE;

	TMvec3 d = tmAddVec3(&a, &b);

	insertVec3(itr, c);
	insertVec3(itr, color);
	insertVec3(itr, tmAddVec3(&c, &a));
	insertVec3(itr, color);

	insertVec3(itr, tmAddVec3(&c, &b));
	insertVec3(itr, color);
	insertVec3(itr, tmAddVec3(&c, &d));
	insertVec3(itr, color);

	indices.push_back(VERTEX_START + 0);
	indices.push_back(VERTEX_START + 1);
	indices.push_back(VERTEX_START + 2);
	indices.push_back(VERTEX_START + 2);
	indices.push_back(VERTEX_START + 1);
	indices.push_back(VERTEX_START + 3);

	return newFragment(SIZE, 6, GL_TRIANGLES);
}

void Geometry::transform(int start, int count, TMmat4 m)
{
	int formatSize = graphics::getSize(VERTEX_COLOR);
	const int LEN = (start + count) * formatSize;
	for (int i = start * formatSize; i < LEN; i += formatSize) {
		TMvec3 v = {vertices[i], vertices[i+1], vertices[i+2]};
		tmTransform(&v, &m, 1.0f);
		vertices[i+0] = v.x;
		vertices[i+1] = v.y;
		vertices[i+2] = v.z;
	}
}

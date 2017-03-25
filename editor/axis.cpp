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

#include "axis.h"
#include <vector>
#include <limits>

using namespace treemaker;

void Axis::create(Geometry &geom)
{
	graphics::Fragment fragment;

	std::vector<Vec3> line(2);
	line[0] = {lineLength[0], 0.0f, 0.0f};
	line[1] = {lineLength[1], 0.0f, 0.0f};
	lines = geom.addLine(line, {1.0f, 0.2f, 0.0f});
	line[0] = {0.0f, lineLength[0], 0.0f};
	line[1] = {0.0f, lineLength[1], 0.0f};
	fragment = geom.addLine(line, {0.0f, 1.0f, 0.2f});
	lines.count[0] += fragment.count[0];
	line[0] = {0.0f, 0.0f, lineLength[0]};
	line[1] = {0.0f, 0.0f, lineLength[1]};
	fragment = geom.addLine(line, {0.0f, 0.2f, 1.0f});
	lines.count[0] += fragment.count[0];

	arrows = geom.addCone(radius, coneLength[0], 10, {0.0f, 1.0f, 0.2f});
	geom.transform(arrows.start[0], arrows.count[0], {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, coneLength[1] - coneLength[0], 0.0f, 1.0f
	});

	fragment = geom.addCone(radius, coneLength[0], 10, {0.0f, 0.2f, 1.0f});
	geom.transform(fragment.start[0], fragment.count[0], {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, coneLength[1] - coneLength[0], 1.0f
	});
	arrows.count[0] += fragment.count[0];
	arrows.count[1] += fragment.count[1];

	fragment = geom.addCone(radius, coneLength[0], 10, {1.0f, 0.2f, 0.0f});
	geom.transform(fragment.start[0], fragment.count[0], {
		0.0f, -1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		coneLength[1] - coneLength[0], 0.0f, 0.0f, 1.0f
	});
	arrows.count[0] += fragment.count[0];
	arrows.count[1] += fragment.count[1];

	lines.type = GL_LINES;
}

void Axis::setScale(float height)
{
	viewportScale = 600.0f / height;
}

Axis::Name Axis::pickClosest(float t[3])
{
	float smallest = std::numeric_limits<float>::max();
	Name selected = NONE;

	for (int i = 0; i < 3; i++)
		if (t[i] != 0.0f)
			if (t[i] < smallest) {
				smallest = t[i];
				selected = static_cast<Name>(i);
			}

	return selected;
}

void Axis::pickCenter()
{
	lastSelected = CENTER;
}

Axis::Name Axis::pickAxis(Vec3 center, Ray ray)
{
	Aabb box;
	float scale = magnitude(ray.origin - center) / 15.0f * viewportScale;
	float t[3];

	if (intersectsSphere(ray, center, 0.5f * scale) > 0.0f) {
		lastSelected = CENTER;
		return lastSelected;
	}

	box.a.x = coneLength[0] * scale + center.x;
	box.b.x = coneLength[1] * scale + center.x;
	box.a.y = -radius * scale + center.y;
	box.b.y = radius * scale + center.y;
	box.a.z = -radius * scale + center.z;
	box.b.z = radius * scale + center.z;
	t[0] = intersectsAABB(ray, box);

	box.a.y = coneLength[0] * scale + center.y;
	box.b.y = coneLength[1] * scale + center.y;
	box.a.x = -radius * scale + center.x;
	box.b.x = radius * scale + center.x;
	box.a.z = -radius * scale + center.z;
	box.b.z = radius * scale + center.z;
	t[1] = intersectsAABB(ray, box);

	box.a.z = coneLength[0] * scale + center.z;
	box.b.z = coneLength[1] * scale + center.z;
	box.a.x = -radius * scale + center.x;
	box.b.x = radius * scale + center.x;
	box.a.y = -radius * scale + center.y;
	box.b.y = radius * scale + center.y;
	t[2] = intersectsAABB(ray, box);

	lastSelected = pickClosest(t);
	return lastSelected;
}

Mat4 Axis::getModelMatrix(Vec3 center, Vec3 position)
{
	float m = magnitude(position - center) / 15.0f * viewportScale;
	return (Mat4){
		m, 0.0f, 0.0f, 0.0f,
		0.0f, m, 0.0f, 0.0f,
		0.0f, 0.0f, m, 0.0f,
		center.x, center.y, center.z, 1.0f
	};
}

graphics::Fragment Axis::getLineFragment()
{
	return lines;
}

graphics::Fragment Axis::getArrowFragment()
{
	return arrows;
}

Vec3 Axis::move(Axis::Name axis, Ray ray, Vec3 direction, Vec3 point)
{
	float distance;
	Plane plane = {point, direction};

	switch (axis) {
	case X_AXIS:
		plane.normal.x = 0.0f;
		break;
	case Y_AXIS:
		plane.normal.y = 0.0f;
		break;
	case Z_AXIS:
		plane.normal.z = 0.0f;
		break;
	default:
		break;
	}

	plane.normal = normalize(plane.normal);

	plane.normal.x = -plane.normal.x;
	plane.normal.y = -plane.normal.y;
	plane.normal.z = -plane.normal.z;

	distance = intersectsPlane(ray, plane);
	point = distance * ray.direction + ray.origin;

	switch (axis) {
	case X_AXIS:
		point.y = plane.point.y;
		point.z = plane.point.z;
		break;
	case Y_AXIS:
		point.x = plane.point.x;
		point.z = plane.point.z;
		break;
	case Z_AXIS:
		point.x = plane.point.x;
		point.y = plane.point.y;
		break;
	default:
		break;
	}

	return point;
}

Axis::Name Axis::getSelected()
{
	return lastSelected;
}

void Axis::clearSelected()
{
	lastSelected = NONE;
}

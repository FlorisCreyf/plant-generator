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

void Axis::create(Geometry &geom)
{
	graphics::Fragment fragment;

	std::vector<TMvec3> line(2);
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

Axis::Name Axis::pickAxis(TMvec3 center, TMray ray)
{
	TMaabb box;
	TMvec3 diff = tmSubVec3(&ray.origin, &center);
	float scale = tmMagVec3(&diff) / 15.0f * viewportScale;
	float t[3];

	if (tmIntersectsSphere(ray, center, 0.5f * scale) > 0.0f) {
		lastSelected = CENTER;
		return lastSelected;
	}

	box.x1 = coneLength[0] * scale + center.x;
	box.x2 = coneLength[1] * scale + center.x;
	box.y1 = -radius * scale + center.y;
	box.y2 = radius * scale + center.y;
	box.z1 = -radius * scale + center.z;
	box.z2 = radius * scale + center.z;
	t[0] = tmIntersectsAABB(ray.origin, ray.direction, box);

	box.y1 = coneLength[0] * scale + center.y;
	box.y2 = coneLength[1] * scale + center.y;
	box.x1 = -radius * scale + center.x;
	box.x2 = radius * scale + center.x;
	box.z1 = -radius * scale + center.z;
	box.z2 = radius * scale + center.z;
	t[1] = tmIntersectsAABB(ray.origin, ray.direction, box);

	box.z1 = coneLength[0] * scale + center.z;
	box.z2 = coneLength[1] * scale + center.z;
	box.x1 = -radius * scale + center.x;
	box.x2 = radius * scale + center.x;
	box.y1 = -radius * scale + center.y;
	box.y2 = radius * scale + center.y;
	t[2] = tmIntersectsAABB(ray.origin, ray.direction, box);

	lastSelected = pickClosest(t);
	return lastSelected;
}

TMmat4 Axis::getModelMatrix(TMvec3 center, TMvec3 position)
{
	TMvec3 a = tmSubVec3(&position, &center);
	float m = tmMagVec3(&a) / 15.0f * viewportScale;
	return (TMmat4){
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

TMvec3 Axis::move(Axis::Name axis, TMray ray, TMvec3 direction, TMvec3 point)
{
	float distance;
	TMplane plane = {point, direction};

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

	tmNormalizeVec3(&plane.normal);

	plane.normal.x = -plane.normal.x;
	plane.normal.y = -plane.normal.y;
	plane.normal.z = -plane.normal.z;

	distance = tmIntersectsPlane(ray, plane);
	point = tmMultVec3(distance, &ray.direction);
	point = tmAddVec3(&ray.origin, &point);

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

Axis::Name Axis::getLastSelected()
{
	return lastSelected;
}

void Axis::clearLastSelected()
{
	lastSelected = NONE;
}

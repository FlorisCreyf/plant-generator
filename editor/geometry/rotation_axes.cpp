/* Plant Genererator
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

#include "rotation_axes.h"

Geometry RotationAxes::getLines()
{
	unsigned index = 0;
	float radius = 0.8f;
	unsigned resolution = 20;
	Geometry lines;
	Geometry::Segment segment;

	lines.addCircle(radius, resolution, {0.0f, 1.0f, 0.2f});
	while (index < resolution)
		lines.addIndex(index++);
	lines.addIndex(index - resolution);
	lines.addIndex(Geometry::primitiveReset);
	segment = lines.getSegment();

	lines.addCircle(radius, resolution, {0.0f, 0.2f, 1.0f});
	lines.transform(segment.pcount, segment.pcount, {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	});
	while (index < resolution*2)
		lines.addIndex(index++);
	lines.addIndex(index - resolution);
	lines.addIndex(Geometry::primitiveReset);

	lines.addIndex(Geometry::primitiveReset);
	lines.addCircle(radius, resolution, {1.0f, 0.2f, 0.0f});
	lines.transform(segment.pcount*2, segment.pcount, {
		0.0f, -1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	});
	while (index < resolution*3)
		lines.addIndex(index++);
	lines.addIndex(index - resolution);

	return lines;
}

void RotationAxes::selectAxis(Axis axis)
{
	this->selection = axis;
}

pg::Mat4 RotationAxes::rotate(pg::Ray ray, pg::Vec3 cameraDirection,
	pg::Vec3 direction)
{
	float distance;
	cameraDirection = pg::normalize(cameraDirection);
	cameraDirection.x = -cameraDirection.x;
	cameraDirection.y = -cameraDirection.y;
	cameraDirection.z = -cameraDirection.z;
	pg::Plane plane = {position, cameraDirection};

	pg::Vec3 normal;
	switch (selection) {
	case XAxis:
		if (ray.origin.x < 0.0f)
			normal.x = 1.0f;
		else
			normal.x = -1.0f;
		normal.y = 0.0f;
		normal.z = 0.0f;
		break;
	case YAxis:
		normal.x = 0.0f;
		if (ray.origin.y < 0.0f)
			normal.y = 1.0f;
		else
			normal.y = -1.0f;
		normal.z = 0.0f;
		break;
	case ZAxis:
		normal.x = 0.0f;
		normal.y = 0.0f;
		if (ray.origin.z < 0.0f)
			normal.z = 1.0f;
		else
			normal.z = -1.0f;
		break;
	default:
		normal = plane.normal;
		break;
	}

	distance = pg::intersectsPlane(ray, plane);
	pg::Vec3 p = (distance * ray.direction + ray.origin);

	pg::Mat4 t = pg::rotateIntoVec(cameraDirection, normal);
	plane.normal = toVec3(t * toVec4(plane.normal, 0.0f));
	p = toVec3(t * toVec4(p - position, 1.0f)) + position;

	float l = pg::project(direction, -1.0f * plane.normal);
	pg::Vec3 p0 = (-1.0f * l * plane.normal) + position;
	float m = pg::magnitude(position+direction - p0);
	pg::Vec3 p2 = m * pg::normalize(p+(-1.0f * l * plane.normal) - p0);
	return pg::rotateIntoVec(direction, normalize(p2+p0 - position));
}

pg::Mat4 RotationAxes::getTransformation(pg::Vec3 cameraPosition,
	pg::Vec3 direction)
{
	float m = pg::magnitude(cameraPosition - position) / 15.0f * scale;
	pg::Mat4 transform = {
		m, 0.0f, 0.0f, 0.0f,
		0.0f, m, 0.0f, 0.0f,
		0.0f, 0.0f, m, 0.0f,
		position.x, position.y, position.z, 1.0f
	};
	return transform * pg::rotateIntoVec({0.0f, 1.0f, 0.0f}, direction);
}

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

#include "axes.h"
#include <limits>

Geometry Axes::getLines()
{
	Geometry lines;
	pg::Vec3 color;
	pg::Vec3 line[2];

	line[0] = {lineLength[0], 0.0f, 0.0f};
	line[1] = {lineLength[1], 0.0f, 0.0f};
	color = {1.0f, 0.2f, 0.0f};
	lines.addLine(line, color);

	line[0] = {0.0f, lineLength[0], 0.0f};
	line[1] = {0.0f, lineLength[1], 0.0f};
	color = {0.0f, 1.0f, 0.2f};
	lines.addLine(line, color);

	line[0] = {0.0f, 0.0f, lineLength[0]};
	line[1] = {0.0f, 0.0f, lineLength[1]};
	color = {0.0f, 0.2f, 1.0f};
	lines.addLine(line, color);

	return lines;
}

Geometry Axes::getArrows()
{
	Geometry::Segment segment;
	Geometry arrows;
	pg::Vec3 color;
	int size = 10;

	color = {0.0f, 1.0f, 0.2f};
	arrows.addCone(radius, coneLength[0], size, color);
	segment = arrows.getSegment();
	arrows.transform(segment.pstart, segment.pcount, {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, coneLength[1] - coneLength[0], 0.0f, 1.0f
	});

	color = {0.0f, 0.2f, 1.0f};
	arrows.addCone(radius, coneLength[0], size, color);
	arrows.transform(segment.pcount, segment.pcount, {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, coneLength[1] - coneLength[0], 1.0f
	});

	color = {1.0f, 0.2f, 0.0f};
	arrows.addCone(radius, coneLength[0], size, color);
	arrows.transform(segment.pcount*2, segment.pcount, {
		0.0f, -1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		coneLength[1] - coneLength[0], 0.0f, 0.0f, 1.0f
	});

	return arrows;
}

void Axes::setScale(float scale)
{
	this->scale = scale;
}

void Axes::setPosition(pg::Vec3 position)
{
	this->position = position;
}

pg::Vec3 Axes::getPosition()
{
	return position;
}

void Axes::selectCenter()
{
	selection = Center;
}

int getClosest(float t[3])
{
	float smallest = std::numeric_limits<float>::max();
	int selected = 0;

	for (int i = 0; i < 3; i++)
		if (t[i] != 0.0f)
			if (t[i] < smallest) {
				smallest = t[i];
				selected = i + 1;
			}

	return selected;
}

Axes::Axis Axes::selectAxis(pg::Ray ray)
{
	pg::Aabb box;
	float scale = pg::magnitude(ray.origin - position)/15.0f * this->scale;
	float t[3];

	if (pg::intersectsSphere(ray, position, 0.5f * scale) > 0.0f) {
		selection = Center;
		return selection;
	} else {
		box.a.x = coneLength[0] * scale + position.x;
		box.b.x = coneLength[1] * scale + position.x;
		box.a.y = -radius * scale + position.y;
		box.b.y = radius * scale + position.y;
		box.a.z = -radius * scale + position.z;
		box.b.z = radius * scale + position.z;
		t[0] = pg::intersectsAABB(ray, box);

		box.a.y = coneLength[0] * scale + position.y;
		box.b.y = coneLength[1] * scale + position.y;
		box.a.x = -radius * scale + position.x;
		box.b.x = radius * scale + position.x;
		box.a.z = -radius * scale + position.z;
		box.b.z = radius * scale + position.z;
		t[1] = pg::intersectsAABB(ray, box);

		box.a.z = coneLength[0] * scale + position.z;
		box.b.z = coneLength[1] * scale + position.z;
		box.a.x = -radius * scale + position.x;
		box.b.x = radius * scale + position.x;
		box.a.y = -radius * scale + position.y;
		box.b.y = radius * scale + position.y;
		t[2] = pg::intersectsAABB(ray, box);

		selection = static_cast<Axis>(getClosest(t));
	}

	return selection;
}

Axes::Axis Axes::getSelection()
{
	return selection;
}

void Axes::clearSelection()
{
	selection = None;
}

pg::Vec3 Axes::move(pg::Ray ray, pg::Vec3 cameraDirection)
{
	float distance;
	pg::Plane plane = {position, cameraDirection};

	switch (selection) {
	case XAxis:
		plane.normal.x = 0.0f;
		break;
	case YAxis:
		plane.normal.y = 0.0f;
		break;
	case ZAxis:
		plane.normal.z = 0.0f;
		break;
	default:
		break;
	}

	plane.normal = pg::normalize(plane.normal);
	plane.normal.x = -plane.normal.x;
	plane.normal.y = -plane.normal.y;
	plane.normal.z = -plane.normal.z;
	distance = pg::intersectsPlane(ray, plane);
	position = distance * ray.direction + ray.origin;

	switch (selection) {
	case XAxis:
		position.y = plane.point.y;
		position.z = plane.point.z;
		break;
	case YAxis:
		position.x = plane.point.x;
		position.z = plane.point.z;
		break;
	case ZAxis:
		position.x = plane.point.x;
		position.y = plane.point.y;
		break;
	default:
		break;
	}

	return position;
}

pg::Mat4 Axes::getTransformation(pg::Vec3 cameraPosition)
{
	float m = pg::magnitude(cameraPosition - position) / 15.0f * scale;
	pg::Mat4 transform = {
		m, 0.0f, 0.0f, 0.0f,
		0.0f, m, 0.0f, 0.0f,
		0.0f, 0.0f, m, 0.0f,
		position.x, position.y, position.z, 1.0f
	};
	return transform;
}

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

#include "translation_axes.h"
#include <limits>

using pg::Vec3;
using pg::Mat4;

Geometry TranslationAxes::getLines()
{
	Geometry lines;
	Vec3 color;
	Vec3 line[2];

	line[0] = Vec3(lineLength[0], 0.0f, 0.0f);
	line[1] = Vec3(lineLength[1], 0.0f, 0.0f);
	color = Vec3(1.0f, 0.2f, 0.0f);
	lines.addLine(line, color);

	line[0] = Vec3(0.0f, lineLength[0], 0.0f);
	line[1] = Vec3(0.0f, lineLength[1], 0.0f);
	color = Vec3(0.0f, 1.0f, 0.2f);
	lines.addLine(line, color);

	line[0] = Vec3(0.0f, 0.0f, lineLength[0]);
	line[1] = Vec3(0.0f, 0.0f, lineLength[1]);
	color = Vec3(0.0f, 0.2f, 1.0f);
	lines.addLine(line, color);

	return lines;
}

Geometry TranslationAxes::getArrows()
{
	Geometry::Segment segment;
	Geometry arrows;
	Vec3 color;
	int size = 10;

	color = {0.0f, 1.0f, 0.2f};
	arrows.addCone(radius, coneLength[0], size, color);
	segment = arrows.getSegment();
	arrows.transform(segment.pstart, segment.pcount, Mat4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, coneLength[1] - coneLength[0], 0.0f, 1.0f));

	color = {0.0f, 0.2f, 1.0f};
	arrows.addCone(radius, coneLength[0], size, color);
	arrows.transform(segment.pcount, segment.pcount, Mat4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, coneLength[1] - coneLength[0], 1.0f));

	color = {1.0f, 0.2f, 0.0f};
	arrows.addCone(radius, coneLength[0], size, color);
	arrows.transform(segment.pcount*2, segment.pcount, Mat4(
		0.0f, -1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		coneLength[1] - coneLength[0], 0.0f, 0.0f, 1.0f));

	return arrows;
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

TranslationAxes::Axis TranslationAxes::selectAxis(pg::Ray ray, float distance)
{
	pg::Aabb box;
	float scale = distance / 15.0f * this->scale;
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

pg::Mat4 TranslationAxes::getTransformation(float distance)
{
	float m = 1.0f;
	if (scalable)
		m = distance / 15.0f * scale;
	return Mat4(
		m, 0.0f, 0.0f, 0.0f,
		0.0f, m, 0.0f, 0.0f,
		0.0f, 0.0f, m, 0.0f,
		position.x, position.y, position.z, 1.0f);
}

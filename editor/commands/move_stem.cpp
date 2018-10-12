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

#include "move_stem.h"
#include <limits>

using pg::Vec3;

MoveStem::MoveStem(StemSelection *selection, Camera &camera, int x, int y)
{
	this->selection = selection;
	this->camera = camera;
	snap = false;
	origCursor.x = x;
	origCursor.y = y;

	pg::Vec3 point;
	point.x = x;
	point.y = y;
	point.z = 0.0f;
	auto instances = selection->getInstances();
	for (auto instance : instances) {
		pg::Stem *stem = instance.first;
		pg::Vec3 offset = camera.toScreenSpace(stem->getLocation());
		offset.z = 0.0f;
		offsets.emplace(stem, point - offset);
	}
}

void MoveStem::snapToCursor(bool snap)
{
	this->snap = snap;
}

void MoveStem::set(int x, int y)
{
	this->cursor.x = x;
	this->cursor.y = y;
}

/** Moves a stem along the path of the parent stem. */
void MoveStem::moveAlongPath(pg::Stem *stem)
{
 	float t = 0.0f;
 	size_t line = 0;
	pg::VolumetricPath path = stem->getParent()->getPath();
	Vec3 point;
	point.x = cursor.x;
	point.y = cursor.y;
	point.z = 0.0f;

	if (!snap)
		point -= offsets.at(stem);

	{
		/* Convert the parent path to the screen space. */
		auto points = path.get();
		{
			pg::Vec3 location = stem->getParent()->getLocation();
			for (pg::Vec3 &point : points) {
				point += location;
				point = camera.toScreenSpace(point);
				point.z = 0.0f;
			}
		}

		/* Find a distance along a path segment. */
		float min = std::numeric_limits<float>::max();
		for (size_t i = 0; i < points.size() - 1; i++) {
			Vec3 a = point - points[i];
			Vec3 b = points[i+1] - points[i];
			float dist = pg::project(a, b);

			if (dist < 0.0f)
				dist = 0.0f;
			if (dist > 1.0f)
				dist = 1.0f;

			float mag = pg::magnitude(points[i] + dist * b - point);

			if (min > mag) {
				t = dist;
				min = mag;
				line = i;
			}
		}
	}

	/* Add the distances along each segment until the distance where the
	 * intersection occurred. */
	auto points = path.get();;
 	t *= pg::magnitude(points[line] - points[line+1]);

	float len = 0.0f;
	for (size_t i = 0; i < line; i++)
 		len += pg::magnitude(points[i + 1] - points[i]);
	len += t; /* Order of floating point additions matters here. */

	stem->setPosition(len);
}

void MoveStem::execute()
{
	auto instances = selection->getInstances();
	for (auto &instance : instances) {
		pg::Stem *stem = instance.first;
		if (stem->getParent()) {
			moveAlongPath(stem);
		}
	}
}

void MoveStem::undo()
{
	pg::Vec2 c = cursor;
	cursor = origCursor;
	execute();
	cursor = c; /* Set cursor for redo. */
}

MoveStem *MoveStem::clone()
{
	return new MoveStem(*this);
}

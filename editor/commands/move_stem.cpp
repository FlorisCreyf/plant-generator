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

MoveStem::MoveStem(Selection *selection, Camera &camera, int x, int y)
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

	auto stemInstances = selection->getStemInstances();
	for (auto instance : stemInstances) {
		pg::Stem *stem = instance.first;
		pg::Vec3 offset = camera.toScreenSpace(stem->getLocation());
		offset.z = 0.0f;
		stemOffsets.emplace(stem, point - offset);
	}

	auto leafInstances = selection->getLeafInstances();
	for (auto instance : leafInstances) {
		std::vector<pg::Vec3> offsets;
		pg::Stem *stem = instance.first;
		pg::Path path = stem->getPath();

		for (unsigned id : instance.second) {
			Vec3 location = stem->getLocation();
			pg::Leaf *leaf = stem->getLeaf(id);
			float position = leaf->getPosition();
			if (position >= 0.0f && position < path.getLength())
				location += path.getIntermediate(position);
			else
				location += path.get().back();

			pg::Vec3 offset = camera.toScreenSpace(location);
			offset.z = 0.0f;
			offsets.push_back(point - offset);
		}

		leafOffsets.emplace(stem, offsets);
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

void MoveStem::getPosition(pg::Stem *parent, size_t &line, float &t, Vec3 point)
{
	pg::VolumetricPath path = parent->getPath();

	/* Convert the parent path to the screen space. */
	auto points = path.get();
	{
		pg::Vec3 location = parent->getLocation();
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

float MoveStem::getLength(pg::VolumetricPath path, size_t line, float t)
{
	/* Add the distances along each segment until the distance where the
	 * intersection occurred. */
	auto points = path.get();;
	t *= pg::magnitude(points[line] - points[line+1]);

	float len = 0.0f;
	for (size_t i = 0; i < line; i++)
		len += pg::magnitude(points[i + 1] - points[i]);
	len += t; /* Order of floating point additions matters here. */

	return len;
}

/** Moves a stem along the path of the parent stem. */
void MoveStem::moveAlongPath(pg::Stem *stem)
{
	pg::VolumetricPath path = stem->getParent()->getPath();
	Vec3 point;
	point.x = cursor.x;
	point.y = cursor.y;
	point.z = 0.0f;

	if (!snap)
		point -= stemOffsets.at(stem);

	float t = 0.0f;
 	size_t line = 0;
	getPosition(stem->getParent(), line, t, point);
	stem->setPosition(getLength(path, line, t));
}

void MoveStem::moveLeavesAlongPath()
{
	auto instances = selection->getLeafInstances();
	for (auto &instance : instances) {
		pg::Stem *stem = instance.first;
		pg::VolumetricPath path = stem->getPath();
		std::vector<pg::Vec3> offsets = leafOffsets.at(stem);

		int l = 0;
		for (unsigned id : instance.second) {
			pg::Leaf *leaf = stem->getLeaf(id);
			Vec3 point;
			point.x = cursor.x;
			point.y = cursor.y;
			point.z = 0.0f;

			if (!snap)
				point -= offsets.at(l++);

			float t = 0.0f;
			size_t line = 0;
			getPosition(stem, line, t, point);
			leaf->setPosition(getLength(path, line, t));
		}
	}
}

void MoveStem::execute()
{
	auto instances = selection->getStemInstances();
	for (auto &instance : instances) {
		pg::Stem *stem = instance.first;
		if (stem->getParent()) {
			moveAlongPath(stem);
		}
	}
	moveLeavesAlongPath();
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

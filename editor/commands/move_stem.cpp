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

MoveStem::MoveStem(
	Selection *selection, const Camera *camera,
	int x, int y, bool snap)
{
	this->selection = selection;
	this->camera = camera;
	this->snap = snap;
	snap = false;
	originalCursor.x = x;
	originalCursor.y = y;
	cursor = originalCursor;

	pg::Vec3 point;
	point.x = x;
	point.y = y;
	point.z = 0.0f;

	if (!snap) {
		setStemOffsets(point);
		setLeafOffsets(point);
	}
}

void MoveStem::setStemOffsets(pg::Vec3 point)
{
	auto stemInstances = selection->getStemInstances();
	for (auto instance : stemInstances) {
		pg::Stem *stem = instance.first;
		pg::Vec3 offset = camera->toScreenSpace(stem->getLocation());
		offset.z = 0.0f;
		stemOffsets.emplace(stem, point - offset);
	}
}

void MoveStem::setLeafOffsets(pg::Vec3 point)
{
	auto leafInstances = selection->getLeafInstances();
	for (auto instance : leafInstances) {
		std::vector<pg::Vec3> offsets;
		pg::Stem *stem = instance.first;
		pg::Path path = stem->getPath();

		for (long id : instance.second) {
			Vec3 location = stem->getLocation();
			pg::Leaf *leaf = stem->getLeaf(id);
			float position = leaf->getPosition();
			if (position >= 0.0f && position < path.getLength())
				location += path.getIntermediate(position);
			else
				location += path.get().back();

			pg::Vec3 offset = camera->toScreenSpace(location);
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

void MoveStem::getPosition(pg::Stem *parent, size_t &line, float &t, Vec3 point)
{
	pg::Path path = parent->getPath();

	/* Convert the parent path to the screen space. */
	auto points = path.get();
	{
		pg::Vec3 location = parent->getLocation();
		for (pg::Vec3 &point : points) {
			point += location;
			point = camera->toScreenSpace(point);
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

float MoveStem::getLength(pg::Path path, size_t line, float t)
{
	/* Add the distances along each segment until the distance where the
	intersection occurred. */
	auto points = path.get();
	t *= pg::magnitude(points[line] - points[line+1]);

	float len = t;
	for (size_t i = 0; i < line; i++)
		len += pg::magnitude(points[i + 1] - points[i]);

	return len;
}

/** Moves a stem along the path of the parent stem. */
void MoveStem::moveAlongPath(pg::Stem *stem)
{
	pg::Path path = stem->getParent()->getPath();
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
		pg::Path path = stem->getPath();

		std::vector<pg::Vec3> offsets;
		if (!snap)
			offsets = leafOffsets.at(stem);

		int l = 0;
		for (long id : instance.second) {
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
	auto stemInstances = selection->getStemInstances();
	for (auto &instance : stemInstances) {
		pg::Stem *stem = instance.first;
		if (stem->getParent())
			moveAlongPath(stem);
	}
	moveLeavesAlongPath();
}

bool MoveStem::onMouseMove(QMouseEvent *event)
{
	QPoint point = event->pos();
	this->cursor.x = point.x();
	this->cursor.y = point.y();
	execute();
	return true;
}

bool MoveStem::onMousePress(QMouseEvent *)
{
	done = true;
	return false;
}

void MoveStem::undo()
{
	pg::Vec2 updatedCursor = cursor;
	cursor = originalCursor;
	execute();
	cursor = updatedCursor;
}

void MoveStem::redo()
{
	execute();
}

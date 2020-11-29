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

#include "move_stem.h"
#include <limits>

using pg::Leaf;
using pg::Path;
using pg::Stem;
using pg::Vec3;

MoveStem::MoveStem(Selection *selection, const Camera *camera, int x, int y,
	bool snap) :
	selection(selection),
	camera(*camera),
	cursor(x, y),
	originalCursor(x, y),
	snap(snap)
{
	setStemOffsets(Vec3(x, y, 0.0f));
	setLeafOffsets(Vec3(x, y, 0.0f));
}

void MoveStem::setStemOffsets(Vec3 point)
{
	auto stemInstances = this->selection->getStemInstances();
	for (auto instance : stemInstances) {
		Stem *stem = instance.first;
		Vec3 offset = this->camera.toScreenSpace(stem->getLocation());
		offset.z = 0.0f;
		this->stemOffsets.emplace(stem, point - offset);
	}
}

void MoveStem::setLeafOffsets(Vec3 point)
{
	auto leafInstances = this->selection->getLeafInstances();
	for (auto instance : leafInstances) {
		std::vector<Vec3> offsets;
		Stem *stem = instance.first;
		const Path &path = stem->getPath();

		for (long id : instance.second) {
			Vec3 location = stem->getLocation();
			Leaf *leaf = stem->getLeaf(id);
			float position = leaf->getPosition();
			if (position >= 0.0f && position < path.getLength())
				location += path.getIntermediate(position);
			else
				location += path.get().back();

			Vec3 offset = this->camera.toScreenSpace(location);
			offset.z = 0.0f;
			offsets.push_back(point - offset);
		}

		this->leafOffsets.emplace(stem, offsets);
	}
}

void MoveStem::snapToCursor(bool snap)
{
	this->snap = snap;
}

void MoveStem::getPosition(Stem *parent, size_t &line, float &t, Vec3 point)
{
	const Path &path = parent->getPath();

	/* Convert the parent path to the screen space. */
	auto points = path.get();
	{
		Vec3 location = parent->getLocation();
		for (Vec3 &point : points) {
			point += location;
			point = this->camera.toScreenSpace(point);
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

float MoveStem::getLength(Path path, size_t line, float t)
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
void MoveStem::moveAlongPath(Stem *stem)
{
	const Path &path = stem->getParent()->getPath();
	Vec3 point(this->cursor.x, this->cursor.y, 0.0f);

	if (!this->snap)
		point -= this->stemOffsets.at(stem);

	float t = 0.0f;
	size_t line = 0;
	getPosition(stem->getParent(), line, t, point);
	stem->setDistance(getLength(path, line, t));
}

void MoveStem::moveLeavesAlongPath()
{
	auto instances = this->selection->getLeafInstances();
	for (auto &instance : instances) {
		Stem *stem = instance.first;
		const Path &path = stem->getPath();

		std::vector<Vec3> offsets;
		if (!this->snap)
			offsets = this->leafOffsets.at(stem);

		int l = 0;
		for (long id : instance.second) {
			Leaf *leaf = stem->getLeaf(id);
			Vec3 point(this->cursor.x, this->cursor.y, 0.0f);

			if (!this->snap)
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
	auto stemInstances = this->selection->getStemInstances();
	for (auto &instance : stemInstances) {
		Stem *stem = instance.first;
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
	this->done = true;
	return false;
}

void MoveStem::undo()
{
	pg::Vec2 updatedCursor = cursor;
	this->cursor = this->originalCursor;
	execute();
	this->cursor = updatedCursor;
}

void MoveStem::redo()
{
	execute();
}

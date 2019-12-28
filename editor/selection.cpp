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

#include "selection.h"
#include "plant_generator/math/intersection.h"
#include <limits>
#include <utility>

using pg::Leaf;
using pg::Path;
using pg::Quat;
using pg::Spline;
using pg::Stem;
using pg::Vec3;

Selection::Selection(Camera *camera, pg::Plant *plant, pg::Mesh *mesh)
{
	this->camera = camera;
	this->plant = plant;
	this->mesh = mesh;
}

bool Selection::operator==(const Selection &obj) const
{
	return stems == obj.stems && leaves == obj.leaves;
}

bool Selection::operator!=(const Selection &obj) const
{
	return !(*this == obj);
}

void Selection::removeStem(Stem *stem)
{
	stems.erase(stem);
}

void Selection::removeLeaves()
{
	leaves.clear();
}

void Selection::addStem(Stem *stem)
{
	PointSelection ps(camera);
	stems.emplace(stem, ps);
}

void Selection::addLeaf(Stem *stem, unsigned leaf)
{
	auto it = leaves.find(stem);
	if (it == leaves.end()) {
		std::set<unsigned> ids;
		ids.insert(leaf);
		leaves.emplace(stem, ids);
	} else
		it->second.insert(leaf);
}

void Selection::select(QMouseEvent *event)
{
	if (!selectPoint(event))
		selectStem(event);
}

bool Selection::selectPoint(QMouseEvent *event)
{
	int point = -1;

	/* Try to select a point from an already selected stem. */
	for (auto &instance : stems) {
		Stem *stem = instance.first;
		PointSelection &ps = instance.second;
		Spline spline = stem->getPath().getSpline();
		Vec3 location = stem->getLocation();
		point = ps.selectPoint(event, spline, location);
		if (point >= 0) {
			if (!(event->modifiers() & Qt::ControlModifier)) {
				PointSelection psCopy = ps;
				stems.clear();
				stems.emplace(stem, psCopy);
			}
			break;
		}
	}

	return point >= 0;
}

void Selection::selectStem(QMouseEvent *event)
{
	QPoint point = event->pos();
	pg::Ray ray = camera->getRay(point.x(), point.y());
	std::pair<float, Stem *> sp = getStem(ray, plant->getRoot());
	std::pair<float, pg::Segment> lp = getLeaf(ray);

	/* Remove previous selections if no modifier key is pressed. */
	if (!(event->modifiers() & Qt::ControlModifier)) {
		stems.clear();
		leaves.clear();
	}

	if (sp.second && sp.first < lp.first) {
		auto it = stems.find(sp.second);
		if (it != stems.end())
			stems.erase(it);
		else
			addStem(sp.second);
	} else if (lp.second.stem && lp.first < sp.first) {
		unsigned leaf = lp.second.leaf;
		Stem *stem = lp.second.stem;
		auto it = leaves.find(stem);
		if (it != leaves.end()) {
			auto leafIt = it->second.find(leaf);
			if (leafIt != it->second.end())
				it->second.erase(leafIt);
			else
				addLeaf(stem, leaf);
		} else
			addLeaf(stem, leaf);
	} else {
		/* Clear the selection if nothing was clicked on. */
		stems.clear();
		leaves.clear();
	}
}

void Selection::setInstances(std::map<Stem *, PointSelection> instances)
{
	this->stems = instances;
}

std::map<Stem *, PointSelection> Selection::getStemInstances()
{
	return stems;
}

std::map<Stem *, std::set<unsigned>> Selection::getLeafInstances()
{
	return leaves;
}

pg::Plant *Selection::getPlant()
{
	return plant;
}

/**
 * Performs cylinder intersection tests to determine which stem was clicked on.
 * A stem and the distance to its intersection is returned.
 */
std::pair<float, Stem *> Selection::getStem(pg::Ray &ray, Stem *stem)
{
	float max = std::numeric_limits<float>::max();
	std::pair<float, Stem *> selection1(max, nullptr);
	std::pair<float, Stem *> selection2(max, nullptr);

	if (stem != nullptr) {
		Path path = stem->getPath();

		for (int i = 0, j = 1; i < path.getSize()-1; i++, j++) {
			Vec3 direction = path.getDirection(i);
			Vec3 line[2] = {path.get(i), path.get(j)};
			line[0] = line[0] + stem->getLocation();
			line[1] = line[1] + stem->getLocation();
			float length = pg::magnitude(line[1] - line[0]);
			float r[2] = {path.getRadius(i), path.getRadius(j)};
			float t = pg::intersectsTaperedCylinder(
				ray, line[0], direction, length, r[0], r[1]);
			if (t > 0 && selection1.first > t) {
				selection1.first = t;
				selection1.second = stem;
			}
		}

		selection2 = getStem(ray, stem->getChild());
		if (selection2.second && selection2.first < selection1.first)
			selection1 = selection2;
		selection2 = getStem(ray, stem->getSibling());
		if (selection2.second && selection2.first < selection1.first)
			selection1 = selection2;
	}

	return selection1;
}

std::pair<float, pg::Segment> Selection::getLeaf(pg::Ray ray)
{
	unsigned indexOffset = 0;
	unsigned vertexOffset = 0;

	std::pair<float, pg::Segment> selection;
	selection.first = std::numeric_limits<float>::max();
	selection.second.stem = nullptr;

	for (int m = 0; m < mesh->getMeshCount(); m++) {
		auto vertices = mesh->getVertices(m);
		auto indices = mesh->getIndices(m);
		auto leaves = mesh->getLeaves(m);
		for (auto pair : leaves) {
			pg::Segment segment = pair.second;
			segment.indexStart -= indexOffset;

			unsigned len = segment.indexStart + segment.indexCount;
			for (unsigned i = segment.indexStart; i < len; i += 3) {
				unsigned triangle[3];
				triangle[0] = (*indices)[i] - vertexOffset;
				triangle[1] = (*indices)[i+1] - vertexOffset;
				triangle[2] = (*indices)[i+2] - vertexOffset;

				Vec3 v1 = (*vertices)[triangle[0]].position;
				Vec3 v2 = (*vertices)[triangle[1]].position;
				Vec3 v3 = (*vertices)[triangle[2]].position;

				float minDistance = selection.first;
				float distance = pg::intersectsTriangle(
					ray, v1, v2, v3);
				if (distance > 0 && distance < minDistance) {
					selection.first = distance;
					selection.second = segment;
				}
			}
		}
		indexOffset += indices->size();
		vertexOffset += vertices->size();
	}
	return selection;
}

void Selection::clear()
{
	stems.clear();
	leaves.clear();
}

void Selection::selectSiblings()
{
	for (auto &instance : stems) {
		Stem *parent = instance.first->getParent();
		if (parent) {
			Stem *child = parent->getChild();
			std::vector<Stem *>::iterator it;
			while (child) {
				PointSelection ps(camera);
				stems.emplace(child, ps);
				child = child->getSibling();
			}
		}
	}
}

void Selection::selectChildren()
{
	for (auto &instance : stems) {
		Stem *child = instance.first->getChild();
		std::vector<Stem *>::iterator it;
		while (child) {
			stems.emplace(child, PointSelection(camera));
			child = child->getSibling();
		}
	}
}

void Selection::reduceToAncestors()
{
	std::map<Stem *, PointSelection> newSelection;
	for (auto &instance : stems) {
		Stem *stem1 = instance.first;
		bool valid = true;
		for (auto instance : stems) {
			Stem *stem2 = instance.first;
			if (stem1->isDescendantOf(stem2)) {
				valid = false;
				break;
			}
		}
		if (valid)
			newSelection.emplace(instance.first, instance.second);
	}
	stems = newSelection;
}

void Selection::selectAll(Stem *stem)
{
	if (stem) {
		Stem *child = stem->getChild();
		while (child) {
			stems.emplace(child, PointSelection(camera));
			selectAll(child);
			child = child->getSibling();
		}
	}
}

void Selection::selectAll()
{
	if (stems.empty()) {
		Stem *stem = plant->getRoot();
		stems.emplace(stem, PointSelection(camera));
		selectAll(stem);
	} else
		stems.clear();
}

bool Selection::hasStems() const
{
	return !stems.empty();
}

bool Selection::hasLeaves() const
{
	return !leaves.empty();
}

bool Selection::contains(Stem *stem) const
{
	return stems.find(stem) != stems.end();
}

void Selection::clearPoints()
{
	for (auto &instance : stems)
		instance.second.clear();
}

void Selection::selectNextPoints()
{
	for (auto &instance : stems) {
		int size = instance.first->getPath().getSpline().getSize();
		instance.second.selectNext(size);
	}
}

void Selection::selectPreviousPoints()
{
	for (auto &instance : stems)
		instance.second.selectPrevious();
}

void Selection::selectAllPoints()
{
	if (hasPoints()) {
		for (auto &instance : stems)
			instance.second.clear();
	} else {
		for (auto &instance : stems) {
			Stem *stem = instance.first;
			int size = stem->getPath().getSpline().getSize();
			instance.second.selectAll(size);
		}
	}
}

void Selection::selectFirstPoints()
{
	for (auto &instance : stems)
		instance.second.select(0);
}

void Selection::selectLastPoints()
{
	for (auto &instance : stems) {
		int size = instance.first->getPath().getSpline().getSize();
		instance.second.select(size - 1);
	}
}

void Selection::getTotalLeafPosition(Vec3 &position, int &count) const
{
	for (auto &instance : leaves) {
		Stem *stem = instance.first;
		Path path = stem->getPath();
		for (unsigned id : instance.second) {
			Leaf *leaf = stem->getLeaf(id);
			position += stem->getLocation();
			position += path.getIntermediate(leaf->getPosition());
			count++;
		}
	}
}

Vec3 Selection::getAveragePosition() const
{
	Vec3 position = {0.0f, 0.0f, 0.0f};
	int count = 0;
	for (auto &instance : stems) {
		const PointSelection &ps = instance.second;
		Stem *stem = instance.first;
		int size = instance.second.getPoints().size();
		if (size > 0) {
			Spline spline = stem->getPath().getSpline();
			Vec3 l = stem->getLocation();
			position += ps.getAveragePosition(spline, l);
			count++;
		}
	}
	getTotalLeafPosition(position, count);
	position /= count;
	return position;
}

Vec3 Selection::getAveragePositionFP() const
{
	Vec3 position = {0.0f, 0.0f, 0.0f};
	int count = 0;
	for (auto &instance : stems) {
		Stem *stem = instance.first;
		int size = instance.second.getPoints().size();
		if (size > 0) {
			int point = *instance.second.getPoints().begin();
			Spline spline = stem->getPath().getSpline();
			Vec3 l = stem->getLocation();
			position += l + spline.getControls()[point];
			count++;
		} else {
			position += stem->getLocation();
			count++;
		}
	}
	getTotalLeafPosition(position, count);
	position /= count;
	return position;
}

Vec3 Selection::getAverageDirectionFP() const
{
	Vec3 dir = {0.0f, 0.0f, 0.0f};
	int count = 0;

	for (auto &instance : stems) {
		Stem *stem = instance.first;
		const PointSelection &ps = instance.second;
		int size = ps.getPoints().size();
		if (size > 0) {
			int point = *ps.getPoints().begin();
			dir += stem->getPath().getSpline().getDirection(point);
			count++;
		} else {
			dir += stem->getPath().getSpline().getDirection(0);
			count++;
		}
	}

	for (auto &instance : leaves) {
		Stem *stem = instance.first;
		for (unsigned id : instance.second) {
			Leaf *leaf = stem->getLeaf(id);
			Quat q = leaf->getRotation();
			Quat k = {0.0f, 0.0f, 1.0f, 0.0f};
			dir += pg::toVec3(q * k * pg::conjugate(q));
			count++;
		}
	}

	dir /= count;
	return pg::normalize(dir);
}

bool Selection::hasPoints() const
{
	bool empty = true;
	for (auto &instance : stems) {
		if (instance.second.hasPoints()) {
			empty = false;
			break;
		}
	}
	return !empty;
}

bool Selection::hasPoint(int point) const
{
	for (auto &instance : stems) {
		auto points = instance.second.getPoints();
		if (points.find(point) != points.end())
			return true;
	}
	return false;
}

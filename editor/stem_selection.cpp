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

#include "stem_selection.h"
#include "plant_generator/math/intersection.h"
#include <limits>
#include <utility>

using pg::Stem;

StemSelection::StemSelection(Camera *camera, pg::Plant *plant)
{
	this->camera = camera;
	this->plant = plant;
}

bool StemSelection::operator==(const StemSelection &obj) const
{
	for (auto &instance : selection) {
		pg::Stem *stem = instance.first;
		auto a = obj.selection.find(stem);
		if (a == obj.selection.end())
			return false;
		if (a->second != instance.second)
			return false;
	}
	return selection.size() == obj.selection.size();
}

bool StemSelection::operator!=(const StemSelection &obj) const
{
	return !(*this == obj);
}

void StemSelection::removeStem(pg::Stem *stem)
{
	selection.erase(stem);
}

void StemSelection::addStem(pg::Stem *stem)
{
	PointSelection ps(camera);
	selection.emplace(stem, ps);
}

void StemSelection::select(QMouseEvent *event)
{
	int point = -1;

	for (auto &instance : selection) {
		Stem *stem = instance.first;
		PointSelection &ps = instance.second;
		pg::Spline spline = stem->getPath().getSpline();
		pg::Vec3 location = stem->getLocation();
		point = ps.selectPoint(event, spline, location);
		if (point >= 0) {
			if (!(event->modifiers() & Qt::ControlModifier)) {
				PointSelection nps = ps;
				selection.clear();
				selection.emplace(stem, nps);
			}
			break;
		}
	}

	if (point < 0)
		selectStem(event);
}

void StemSelection::selectStem(QMouseEvent *event)
{
	QPoint point = event->pos();
	pg::Ray ray = camera->getRay(point.x(), point.y());
	Stem *stem = getStem(ray);

	/* Remove previous selections if no modifier key is pressed. */
	if (!(event->modifiers() & Qt::ControlModifier))
		selection.clear();

	if (stem) {
		/* Remove the stem from the selection if it is already
		 * selected. */
		auto it = selection.find(stem);
		if (it != selection.end())
			selection.erase(it);
		else
			addStem(stem);
	} else
		/* Remove the entire selection if nothing was clicked on. */
		selection.clear();
}

void StemSelection::setInstances(std::map<pg::Stem *, PointSelection> instances)
{
	this->selection = instances;
}

std::map<pg::Stem *, PointSelection> StemSelection::getInstances()
{
	return selection;
}

pg::Plant *StemSelection::getPlant()
{
	return plant;
}

Stem *StemSelection::getStem(pg::Ray ray)
{
	Stem *stem = plant->getRoot();
	std::pair<float, Stem *> selection = getStem(ray, stem);
	return selection.second;
}

/**
 * Performs cylinder intersection tests to determine which stem was clicked on.
 * A stem and the distance to its intersection is returned.
 */
std::pair<float, Stem *> StemSelection::getStem(pg::Ray &ray, Stem *stem)
{
	float max = std::numeric_limits<float>::max();
	std::pair<float, Stem *> selection1(max, nullptr);
	std::pair<float, Stem *> selection2(max, nullptr);

	if (stem != nullptr) {
		pg::VolumetricPath path = stem->getPath();

		for (int i = 0, j = 1; i < path.getSize()-1; i++, j++) {
			pg::Vec3 direction = path.getDirection(i);
			pg::Vec3 line[2] = {path.get(i), path.get(j)};
			line[0] = line[0] + stem->getLocation();
			line[1] = line[1] + stem->getLocation();
			float length = pg::magnitude(line[1] - line[0]);
			float r[2] = {path.getRadius(i), path.getRadius(j)};
			float t = pg::intersectsTaperedCylinder(ray, line[0],
				direction, length, r[0], r[1]);
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

void StemSelection::clear()
{
	selection.clear();
}

void StemSelection::selectSiblings()
{
	for (auto &instance : selection) {
		Stem *parent = instance.first->getParent();
		if (parent) {
			Stem *child = parent->getChild();
			std::vector<Stem *>::iterator it;
			while (child) {
				PointSelection ps(camera);
				selection.emplace(child, ps);
				child = child->getSibling();
			}
		}
	}
}

void StemSelection::selectChildren()
{
	for (auto &instance : selection) {
		Stem *child = instance.first->getChild();
		std::vector<Stem *>::iterator it;
		while (child) {
			selection.emplace(child, PointSelection(camera));
			child = child->getSibling();
		}
	}
}

void StemSelection::reduceToAncestors()
{
	std::map<pg::Stem *, PointSelection> newSelection;
	for (auto &instance : selection) {
		pg::Stem *stem1 = instance.first;
		bool valid = true;
		for (auto instance : selection) {
			pg::Stem *stem2 = instance.first;
			if (stem1->isDescendantOf(stem2)) {
				valid = false;
				break;
			}
		}
		if (valid)
			newSelection.emplace(instance.first, instance.second);
	}
	selection = newSelection;
}

void StemSelection::selectAll(Stem *stem)
{
	if (stem) {
		Stem *child = stem->getChild();
		while (child) {
			selection.emplace(child, PointSelection(camera));
			selectAll(child);
			child = child->getSibling();
		}
	}
}

void StemSelection::selectAll()
{
	if (selection.empty()) {
		Stem *stem = plant->getRoot();
		selection.emplace(stem, PointSelection(camera));
		selectAll(stem);
	} else
		selection.clear();
}

bool StemSelection::hasStems() const
{
	return !selection.empty();
}

bool StemSelection::contains(pg::Stem *stem) const
{
	return selection.find(stem) != selection.end();
}

void StemSelection::clearPoints()
{
	for (auto &instance : selection)
		instance.second.clear();
}

void StemSelection::selectNextPoints()
{
	for (auto &instance : selection) {
		int size = instance.first->getPath().getSpline().getSize();
		instance.second.selectNext(size);
	}
}

void StemSelection::selectPreviousPoints()
{
	for (auto &instance : selection)
		instance.second.selectPrevious();
}

void StemSelection::selectAllPoints()
{
	if (hasPoints())
		for (auto &instance : selection)
			instance.second.clear();
	else {
		for (auto &instance : selection) {
			pg::Stem *stem = instance.first;
			int size = stem->getPath().getSpline().getSize();
			instance.second.selectAll(size);
		}
	}
}

void StemSelection::selectFirstPoints()
{
	for (auto &instance : selection) {
		instance.second.select(0);
	}
}

void StemSelection::selectLastPoints()
{
	for (auto &instance : selection) {
		int size = instance.first->getPath().getSpline().getSize();
		instance.second.select(size - 1);
	}
}

pg::Vec3 StemSelection::getAveragePosition() const
{
	pg::Vec3 position = {0.0f, 0.0f, 0.0f};
	int count = 0;
	for (auto &instance : selection) {
		const PointSelection &ps = instance.second;
		pg::Stem *stem = instance.first;
		int size = instance.second.getPoints().size();
		if (size > 0) {
			pg::Spline spline = stem->getPath().getSpline();
			pg::Vec3 l = stem->getLocation();
			position += ps.getAveragePosition(spline, l);
			count++;
		}
	}
	position /= count;
	return position;
}

pg::Vec3 StemSelection::getAveragePositionFP() const
{
	pg::Vec3 position = {0.0f, 0.0f, 0.0f};
	int count = 0;
	for (auto &instance : selection) {
		pg::Stem *stem = instance.first;
		int size = instance.second.getPoints().size();
		if (size > 0) {
			int point = *instance.second.getPoints().begin();
			pg::Spline spline = stem->getPath().getSpline();
			pg::Vec3 l = stem->getLocation();
			position += l + spline.getControls()[point];
			count++;
		} else {
			position += stem->getLocation();
			count++;
		}
	}
	position /= count;
	return position;
}

pg::Vec3 StemSelection::getAverageDirectionFP() const
{
	pg::Vec3 direction = {0.0f, 0.0f, 0.0f};
	int count = 0;
	for (auto &instance : selection) {
		pg::Stem *stem = instance.first;
		const PointSelection &ps = instance.second;
		int size = ps.getPoints().size();
		if (size > 0) {
			int point = *ps.getPoints().begin();
			direction += stem->getPath().getDirection(point);
			count++;
		} else {
			direction += stem->getPath().getDirection(0);
			count++;
		}
	}
	direction /= count;
	return pg::normalize(direction);
}

bool StemSelection::hasPoints() const
{
	bool empty = true;
	for (auto &instance : selection) {
		if (instance.second.hasPoints()) {
			empty = false;
			break;
		}
	}
	return !empty;
}

bool StemSelection::hasPoint(int point) const
{
	for (auto &instance : selection) {
		auto points = instance.second.getPoints();
		if (points.find(point) != points.end())
			return true;
	}
	return false;
}

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

Selection::Selection(pg::Plant *plant)
{
	this->plant = plant;
}

bool Selection::operator==(const Selection &obj) const
{
	return this->stems == obj.stems && this->leaves == obj.leaves;
}

bool Selection::operator!=(const Selection &obj) const
{
	return !(*this == obj);
}

bool Selection::removeStem(Stem *stem)
{
	return this->stems.erase(stem) > 0;
}

bool Selection::removeLeaf(Stem *stem, long leaf)
{
	bool removed = false;
	auto it = this->leaves.find(stem);
	if (it != this->leaves.end()) {
		auto leafIt = it->second.find(leaf);
		if (leafIt != it->second.end()) {
			removed = true;
			it->second.erase(leafIt);
			if (it->second.empty())
				this->leaves.erase(it);
		}
	}
	return removed;
}

void Selection::removeStems()
{
	this->stems.clear();
}

void Selection::removeLeaves()
{
	this->leaves.clear();
}

void Selection::addStem(Stem *stem)
{
	this->stems.emplace(stem, PointSelection());
}

void Selection::addStem(Stem *stem, const PointSelection &selection)
{
	auto result = this->stems.emplace(stem, selection);
	if (!result.second)
		result.first->second = selection;
}

void Selection::addLeaf(Stem *stem, long leaf)
{
	auto it = this->leaves.find(stem);
	if (it == this->leaves.end()) {
		std::set<long> ids;
		ids.insert(leaf);
		this->leaves.emplace(stem, ids);
	} else
		it->second.insert(leaf);
}

void Selection::setInstances(std::map<Stem *, PointSelection> instances)
{
	this->stems = instances;
}

std::map<Stem *, PointSelection> Selection::getStemInstances() const
{
	return this->stems;
}

std::map<Stem *, std::set<long>> Selection::getLeafInstances() const
{
	return this->leaves;
}

pg::Plant *Selection::getPlant() const
{
	return this->plant;
}

void Selection::clear()
{
	this->stems.clear();
	this->leaves.clear();
}

void Selection::selectSiblings()
{
	std::map<Stem *, PointSelection> siblings;
	for (auto &instance : this->stems) {
		Stem *parent = instance.first->getParent();
		if (!parent)
			continue;

		Stem *child = parent->getChild();
		while (child) {
			siblings.emplace(child, PointSelection());
			child = child->getSibling();
		}
	}
	this->stems = siblings;
}

void Selection::selectChildren()
{
	std::map<Stem *, PointSelection> children;
	for (auto &instance : this->stems) {
		Stem *child = instance.first->getChild();
		while (child) {
			children.emplace(child, PointSelection());
			child = child->getSibling();
		}
	}
	this->stems = children;
}

void Selection::reduceToAncestors()
{
	std::map<Stem *, PointSelection> newSelection;
	for (auto &instance : this->stems) {
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
	this->stems = newSelection;
}

void Selection::selectStems(Stem *stem)
{
	if (stem) {
		Stem *child = stem->getChild();
		while (child) {
			this->stems.emplace(
				child, PointSelection());
			selectStems(child);
			child = child->getSibling();
		}
	}
}

void Selection::selectStems()
{
	if (this->stems.empty()) {
		Stem *stem = this->plant->getRoot();
		this->stems.emplace(stem, PointSelection());
		selectStems(stem);
	} else
		this->stems.clear();
}

void Selection::selectLeaves()
{
	for (auto &instance : this->stems) {
		Stem *stem = instance.first;
		for (auto &leaf : stem->getLeaves())
			addLeaf(stem, leaf.first);
	}
}

bool Selection::hasStems() const
{
	return !this->stems.empty();
}

bool Selection::hasLeaves() const
{
	return !this->leaves.empty();
}

bool Selection::contains(Stem *stem) const
{
	return this->stems.find(stem) != this->stems.end();
}

void Selection::clearPoints()
{
	for (auto &instance : this->stems)
		instance.second.clear();
}

void Selection::selectNextPoints()
{
	for (auto &instance : this->stems) {
		int size = instance.first->getPath().getSpline().getSize();
		instance.second.selectNext(size);
	}
}

void Selection::selectPreviousPoints()
{
	for (auto &instance : this->stems)
		instance.second.selectPrevious();
}

void Selection::selectAllPoints()
{
	if (hasPoints()) {
		for (auto &instance : this->stems)
			instance.second.clear();
	} else {
		for (auto &instance : this->stems) {
			Stem *stem = instance.first;
			int size = stem->getPath().getSpline().getSize();
			instance.second.selectAll(size);
		}
	}
}

void Selection::selectFirstPoints()
{
	for (auto &instance : this->stems)
		instance.second.select(0);
}

void Selection::selectLastPoints()
{
	for (auto &instance : this->stems) {
		int size = instance.first->getPath().getSpline().getSize();
		instance.second.select(size - 1);
	}
}

void Selection::getTotalLeafPosition(Vec3 &position, int &count) const
{
	for (auto &instance : this->leaves) {
		Stem *stem = instance.first;
		Path path = stem->getPath();
		for (long id : instance.second) {
			Leaf *leaf = stem->getLeaf(id);
			position += stem->getLocation();
			position += path.getIntermediate(leaf->getPosition());
			count++;
		}
	}
}

Vec3 Selection::getAveragePosition() const
{
	Vec3 position(0.0f, 0.0f, 0.0f);
	int count = 0;
	for (auto &instance : this->stems) {
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
	Vec3 position(0.0f, 0.0f, 0.0f);
	int count = 0;
	for (auto &instance : this->stems) {
		Stem *stem = instance.first;
		int size = instance.second.getPoints().size();
		if (size > 0) {
			int point = *instance.second.getPoints().begin();
			Spline spline = stem->getPath().getSpline();
			Vec3 location = stem->getLocation();
			position += location + spline.getControls()[point];
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
	Vec3 dir(0.0f, 0.0f, 0.0f);
	int count = 0;

	for (auto &instance : this->stems) {
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

	for (auto &instance : this->leaves) {
		Stem *stem = instance.first;
		for (long id : instance.second) {
			Leaf *leaf = stem->getLeaf(id);
			Quat q = leaf->getRotation();
			Quat k(0.0f, 0.0f, 1.0f, 0.0f);
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
	for (auto &instance : this->stems) {
		if (instance.second.hasPoints()) {
			empty = false;
			break;
		}
	}
	return !empty;
}

bool Selection::hasPoint(int point) const
{
	for (auto &instance : this->stems) {
		auto points = instance.second.getPoints();
		if (points.find(point) != points.end())
			return true;
	}
	return false;
}

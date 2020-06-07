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

#include "remove_stem.h"
#include "remove_spline.h"

using pg::Stem;
using pg::Path;
using pg::Spline;
using pg::Plant;

RemoveStem::RemoveStem(Selection *selection) : prevSelection(*selection)
{
	this->selection = selection;
}

RemoveStem::~RemoveStem()
{
	if (!this->stems.empty()) {
		/* Adjust the selection so that descendants are not deleted
		after ancestors have been deleted. */
		this->prevSelection.reduceToAncestors();
		for (auto stem : this->stems)
			delete stem;
	}
}

/** Should be called after stems are removed. Leaves do not need to be removed
from the plant if their stem is removed. */
void RemoveStem::removeLeaves()
{
	auto instances = this->selection->getLeafInstances();
	for (auto &instance : instances) {
		Stem *stem = instance.first;
		std::set<long> &ids = instance.second;
		for (auto it = ids.begin(); it != ids.end(); it++) {
			this->leaves.emplace(stem, *(stem->getLeaf(*it)));
			stem->removeLeaf(*it);
		}
	}
	this->selection->removeLeaves();
}

void RemoveStem::removeStems()
{
	std::vector<Stem *> removals;
	auto instances = this->selection->getStemInstances();
	for (auto &instance : instances) {
		Stem *stem = instance.first;
		Path path = stem->getPath();
		Spline spline = path.getSpline();
		PointSelection &pointSelection = instance.second;
		auto points = pointSelection.getPoints();

		int first = spline.getDegree() == 1 ? 0 : 1;
		if (points.empty() || *points.begin() <= first) {
			/* Remove the whole stem. */
			if (stem->getParent()) {
				points.clear();
				this->selection->getPlant()->extractStem(stem);
				this->stems.push_back(stem);
				removals.push_back(stem);
			}
		} else {
			/* Remove points from the stem. */
			this->splines.emplace(stem, spline);

			RemoveSpline removeSpline(&pointSelection, &spline);
			removeSpline.setClearable(false);
			removeSpline.execute();
			path.setSpline(spline);
			stem->setPath(path);

			if (pointSelection.hasPoints() && stem->getParent()) {
				this->selection->getPlant()->extractStem(stem);
				removals.push_back(stem);
			} else
				pointSelection.clear();
		}
	}

	/* Do not remove stems from the selection if only a couple of points
	were removed. */
	this->selection->setInstances(instances);
	for (auto stem : removals)
		this->selection->removeStem(stem);
}

void RemoveStem::execute()
{
	/* Remove leaves first to avoid checking for deleted stems. */
	removeLeaves();
	removeStems();
}

void RemoveStem::undo()
{
	for (Stem *stem : this->stems) {
		Plant *plant = this->selection->getPlant();
		plant->insertStem(stem, stem->getParent());
	}

	for (auto &pair : this->splines) {
		Stem *stem = pair.first;
		auto path = stem->getPath();
		path.setSpline(pair.second);
		stem->setPath(path);
	}

	for (auto &item : this->leaves) {
		Stem *stem = item.first;
		stem->addLeaf(item.second);
	}

	this->stems.clear();
	this->splines.clear();
	*this->selection = this->prevSelection;
}

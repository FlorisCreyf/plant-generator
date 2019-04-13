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

#include "remove_stem.h"
#include "remove_spline.h"

RemoveStem::RemoveStem(Selection *selection) : prevSelection(*selection)
{
	this->selection = selection;
	cloned = false;

	/* Remember the original splines before removal. */
	auto instances = selection->getStemInstances();
	for (auto &instance : instances) {
		pg::Stem *stem = instance.first;
		pg::VolumetricPath path = stem->getPath();
		pg::Spline spline = path.getSpline();
		splines.emplace(stem, spline);
	}
}

RemoveStem::~RemoveStem()
{
	if (!cloned && !removals.empty()) {
		/* Adjust the selection so that descendants are not deleted
		 * after ancestors have been deleted. */
		prevSelection.reduceToAncestors();
		for (auto stem : removals)
			if (prevSelection.contains(stem))
				pg::Plant::deleteStem(stem);
	}
}

/**
 * Should be called after stems are removed. Leaves do not need to be removed
 * from the plant anymore if their stem is removed.
 */
void RemoveStem::removeLeaves()
{
	auto instances = selection->getLeafInstances();
	for (auto &instance : instances) {
		pg::Stem *stem = instance.first;
		if (selection->getPlant()->contains(stem)) {
			std::set<unsigned> &ids = instance.second;
			for (auto it = ids.begin(); it != ids.end(); it++) {
				leaves.emplace(stem, *(stem->getLeaf(*it)));
				stem->removeLeaf(*it);
			}
		}
	}
	selection->removeLeaves();
}

void RemoveStem::execute()
{
	removals.clear();
	auto instances = selection->getStemInstances();
	for (auto &instance : instances) {
		pg::Stem *stem = instance.first;
		pg::VolumetricPath path = stem->getPath();
		pg::Spline spline = path.getSpline();
		PointSelection &pointSelection = instance.second;
		auto points = pointSelection.getPoints();

		int first = spline.getDegree() == 1 ? 0 : 1;
		if (points.empty() || *points.begin() <= first) {
			if (stem->getParent()) {
				points.clear();
				selection->getPlant()->release(stem);
				removals.push_back(stem);
			}
		} else {
			RemoveSpline rp(&pointSelection, &spline);
			rp.setClearable(false);
			rp.execute();
			path.setSpline(spline);
			stem->setPath(path);

			if (pointSelection.hasPoints() && stem->getParent()) {
				selection->getPlant()->release(stem);
				removals.push_back(stem);
			} else
				pointSelection.clear();
		}
	}

	selection->setInstances(instances);
	for (auto stem : removals)
		selection->removeStem(stem);

	removeLeaves();
}

void RemoveStem::undo()
{
	for (auto &item : splines) {
		pg::Stem *stem = item.first;
		pg::Plant *plant = selection->getPlant();
		if (plant->contains(stem)) {
			auto path = stem->getPath();
			path.setSpline(item.second);
			stem->setPath(path);
		} else {
			plant->insert(stem->getParent(), stem);
		}
	}

	for (auto &item : leaves) {
		pg::Stem *stem = item.first;
		stem->addLeaf(item.second);
	}

	*selection = prevSelection;
}

RemoveStem *RemoveStem::clone()
{
	cloned = true;
	return new RemoveStem(*this);
}

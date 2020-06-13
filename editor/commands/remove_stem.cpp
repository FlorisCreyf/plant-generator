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

using pg::Leaf;
using pg::Path;
using pg::Plant;
using pg::Spline;
using pg::Stem;
using std::pair;

RemoveStem::RemoveStem(Selection *selection) : prevSelection(*selection)
{
	this->selection = selection;
}

/** Should be called after stems are removed. Leaves do not need to be removed
from the plant if their stem is removed. */
void RemoveStem::removeLeaves()
{
	auto instances = this->selection->getLeafInstances();
	for (auto &instance : instances) {
		Stem *stem = instance.first;
		std::set<size_t> &indices = instance.second;
		for (auto it = indices.rbegin(); it != indices.rend(); it++) {
			LeafState state;
			state.stem = stem;
			state.index = *it;
			state.leaf = *(stem->getLeaf(state.index));
			this->leaves.push_back(state);
			stem->removeLeaf(state.index);
		}
	}
	this->selection->removeLeaves();
}

bool shouldRemoveStem(PointSelection &pointSelection, int degree)
{
	auto points = pointSelection.getPoints();
	int first = degree == 1 ? 0 : 1;
	return points.empty() || *points.begin() <= first;
}

void RemoveStem::removeStems()
{
	Plant *plant = this->selection->getPlant();
	this->selection->reduceToAncestors();
	auto instances = this->selection->getStemInstances();
	for (auto it = instances.begin(); it != instances.end(); it++) {
		Stem *stem = it->first;
		Path path = stem->getPath();
		Spline spline = path.getSpline();
		PointSelection &pointSelection = it->second;
		if (shouldRemoveStem(pointSelection, spline.getDegree())) {
			/* Remove the whole stem. */
			plant->extractStems(stem, this->stems);
		} else {
			/* Remove points from the stem. */
			pair<Stem *, Spline> instance(stem, spline);
			this->splines.push_back(instance);

			RemoveSpline removeSpline(&pointSelection, &spline);
			removeSpline.setClearable(false);
			removeSpline.execute();
			path.setSpline(spline);
			stem->setPath(path);

			if (pointSelection.hasPoints())
				plant->extractStems(stem, this->stems);
		}
	}
}

void RemoveStem::execute()
{
	/* Remove leaves first to avoid checking for deleted stems. */
	removeLeaves();
	removeStems();
	this->selection->clear();
}

void RemoveStem::undo()
{
	Plant *plant = this->selection->getPlant();
	plant->reinsertStems(this->stems);

	for (auto &pair : this->splines) {
		Stem *stem = pair.first;
		auto path = stem->getPath();
		path.setSpline(pair.second);
		stem->setPath(path);
	}

	for (auto it = this->leaves.rbegin(); it != this->leaves.rend(); it++) {
		Stem *stem = it->stem;
		stem->insertLeaf(it->leaf, it->index);
	}

	this->stems.clear();
	this->leaves.clear();
	this->splines.clear();
	*this->selection = this->prevSelection;
}

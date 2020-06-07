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

#ifndef STEM_SELECTION_H
#define STEM_SELECTION_H

#include "point_selection.h"
#include "plant_generator/plant.h"
#include "plant_generator/mesh.h"
#include <map>
#include <vector>
#include <set>

class Selection {
	pg::Plant *plant;
	std::map<pg::Stem *, PointSelection> stems;
	std::map<pg::Stem *, std::set<long>> leaves;

	void selectStems(pg::Stem *);
	void getTotalLeafPosition(pg::Vec3 &, int &) const;

public:
	Selection(pg::Plant *plant);
	bool operator==(const Selection &obj) const;
	bool operator!=(const Selection &obj) const;

	bool removeStem(pg::Stem *stem);
	bool removeLeaf(pg::Stem *stem, long leaf);
	void removeStems();
	void addStem(pg::Stem *stem);
	void addStem(pg::Stem *stem, const PointSelection &selection);
	void addLeaf(pg::Stem *stem, long leaf);
	void removeLeaves();

	void setInstances(std::map<pg::Stem *, PointSelection> instances);
	std::map<pg::Stem *, PointSelection> getStemInstances() const;
	std::map<pg::Stem *, std::set<long>> getLeafInstances() const;
	pg::Plant *getPlant() const;

	void clear();
	void selectSiblings();
	void selectChildren();
	void selectAll();
	void selectStems();
	void selectLeaves();
	void reduceToAncestors();

	bool hasStems() const;
	bool hasLeaves() const;
	bool contains(pg::Stem *stem) const;

	void clearPoints();
	void selectNextPoints();
	void selectPreviousPoints();
	void selectAllPoints();
	void selectFirstPoints();
	void selectLastPoints();
	bool hasPoints() const;
	bool hasPoint(int point) const;

	pg::Vec3 getAveragePosition() const;
	/** The average position of the first selected points of each stem. The
	location of a selected stem is included if no points are selected. */
	pg::Vec3 getAveragePositionFP() const;
	/** The average direction of the first selected points of each stem.
	The direction of a selected stem is included if no points are
	selected. */
	pg::Vec3 getAverageDirectionFP() const;
};

#endif /* STEM_SELECTION_H */

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

#include "camera.h"
#include "point_selection.h"
#include "plant_generator/plant.h"
#include "plant_generator/mesh.h"
#include <QtGui/QMouseEvent>
#include <map>
#include <vector>
#include <set>

class Selection {
	Camera *camera;
	pg::Plant *plant;
	pg::Mesh *mesh;
	std::map<pg::Stem *, PointSelection> stems;
	std::map<pg::Stem *, std::set<unsigned>> leaves;

	std::pair<float, pg::Stem *> getStem(pg::Ray &ray, pg::Stem *stem);
	std::pair<float, pg::Segment> getLeaf(pg::Ray ray);
	void selectAll(pg::Stem *stem);
	void getTotalLeafPosition(pg::Vec3 &, int &) const;

public:
	Selection(Camera *camera, pg::Plant *plant, pg::Mesh *mesh);
	bool operator==(const Selection &obj) const;
	bool operator!=(const Selection &obj) const;

	void removeStem(pg::Stem *stem);
	void addStem(pg::Stem *stem);
	void addLeaf(pg::Stem *stem, unsigned leaf);
	void removeLeaves();

	void select(QMouseEvent *event);
	void setInstances(std::map<pg::Stem *, PointSelection> instances);
	std::map<pg::Stem *, PointSelection> getStemInstances();
	std::map<pg::Stem *, std::set<unsigned>> getLeafInstances();
	pg::Plant *getPlant();

	void clear();
	void selectSiblings();
	void selectChildren();
	void selectAll();
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
	/**
	 * The average position of the first selected points of each stem. The
	 * location of a selected stem is included if no points are selected.
	 */
	pg::Vec3 getAveragePositionFP() const;
	/**
	 * The average direction of the first selected points of each stem. The
	 * direction of a selected stem is included if no points are selected.
	 */
	pg::Vec3 getAverageDirectionFP() const;
};

#endif /* STEM_SELECTION_H */

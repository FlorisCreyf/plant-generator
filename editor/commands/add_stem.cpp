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

#include "add_stem.h"
#include "../commands/move_stem.h"
#include "plant_generator/patterns.h"

AddStem::AddStem(StemSelection *selection)
{
	this->selection = selection;
	undone = false;
}

AddStem::~AddStem()
{
	if (undone)
		pg::Plant::deleteStem(stem);
}

void AddStem::create()
{
	auto instances = selection->getInstances();
	if (instances.size() == 1) {
		pg::Stem *parent = (*instances.begin()).first;
		pg::Plant *plant = selection->getPlant();
		stem = plant->addStem(parent);

		pg::VolumetricPath path = stem->getPath();
		pg::Spline spline = path.getSpline();
		std::vector<pg::Vec3> controls;
		controls.push_back(pg::getZeroVec3());
		/* Adjust the second point at a latter time. */
		controls.push_back(pg::getZeroVec3());
		spline.setControls(controls);
		spline.setDegree(1);
		path.setSpline(spline);
		path.setMaxRadius(parent->getPath().getMaxRadius() / 4.0f);
		path.setRadius(pg::getDefaultCurve(0));
		stem->setPath(path);
		stem->setPosition(0.0f);

		selection->clear();
		selection->addStem(stem);
		selection->selectLastPoints();
	}
}

void AddStem::execute()
{
	if (undone) {
		pg::Plant *plant = selection->getPlant();
		plant->insert(stem->getParent(), stem);
		undone = false;
	} else
		create();
}

void AddStem::undo()
{
	pg::Plant *plant = selection->getPlant();
	plant->release(stem);
	selection->clear();
	selection->addStem(stem->getParent());
	undone = true;
}

AddStem *AddStem::clone()
{
	return new AddStem(*this);
}

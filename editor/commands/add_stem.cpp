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
#include "plant_generator/patterns.h"

AddStem::AddStem(
	Selection *selection, TranslationAxes *axes, const Camera *camera,
	int x, int y) :
	prevSelection(*selection),
	moveStem(selection, camera, x, y, true),
	movePath(selection, axes, camera)
{
	this->selection = selection;
	this->undone = false;
}

AddStem::~AddStem()
{
	if (!this->undone)
		delete stem;
}

void AddStem::create()
{
	auto instances = selection->getStemInstances();
	if (instances.size() == 1) {
		pg::Stem *parent = (*instances.begin()).first;
		pg::Plant *plant = selection->getPlant();
		stem = plant->addStem(parent);

		pg::Path parentPath = parent->getPath();
		pg::Path path = stem->getPath();
		pg::Spline spline = path.getSpline();
		std::vector<pg::Vec3> controls;
		controls.push_back(pg::getZeroVec3());
		/* Adjust the second point at a latter time. */
		controls.push_back(pg::getZeroVec3());
		spline.setControls(controls);
		spline.setDegree(1);
		path.setSpline(spline);
		path.setMaxRadius(parentPath.getMaxRadius() / 4.0f);
		path.setMinRadius(parentPath.getMinRadius());
		path.setRadius(pg::getDefaultCurve(0));
		stem->setPath(path);
		stem->setPosition(0.0f);

		selection->clear();
		selection->addStem(stem);
		selection->selectLastPoints();
	}
}

bool AddStem::onMouseMove(QMouseEvent *event)
{
	if (!this->moveStem.isDone())
		return this->moveStem.onMouseMove(event);
	else
		return this->movePath.onMouseMove(event);
}

bool AddStem::onMousePress(QMouseEvent *event)
{
	if (!this->moveStem.isDone())
		return this->moveStem.onMousePress(event);
	if (this->moveStem.isDone()) {
		bool update = this->movePath.onMouseRelease(event);
		this->done = this->movePath.isDone();
		return update;
	}
	return false;
}

bool AddStem::onMouseRelease(QMouseEvent *event)
{
	return onMousePress(event);
}

void AddStem::execute()
{
	prevSelection = *selection;
	create();
	moveStem.execute();
}

void AddStem::undo()
{
	pg::Plant *plant = selection->getPlant();
	plant->extractStem(stem);
	*selection = prevSelection;
	this->undone = true;
}

void AddStem::redo()
{
	pg::Plant *plant = selection->getPlant();
	plant->insertStem(stem, stem->getParent());
	selection->clear();
	selection->addStem(stem);
	selection->selectLastPoints();
	this->undone = false;
}

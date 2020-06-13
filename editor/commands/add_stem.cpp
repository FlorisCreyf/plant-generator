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

#include "add_stem.h"
#include "plant_generator/patterns.h"

using pg::Stem;
using pg::Plant;

AddStem::AddStem(
	Selection *selection, TranslationAxes *axes, const Camera *camera,
	int x, int y) :
	prevSelection(*selection),
	moveStem(selection, camera, x, y, true),
	movePath(selection, axes, camera)
{
	this->selection = selection;
}

void AddStem::create()
{
	Plant *plant = this->selection->getPlant();
	int pathDivisions = 1;
	auto instances = this->selection->getStemInstances();
	if (instances.size() == 1) {
		Stem *parent = (*instances.begin()).first;
		stem = plant->addStem(parent);
		stem->setResolution(parent->getResolution());
		stem->setMaterial(
			Stem::Outer, parent->getMaterial(Stem::Outer));
		stem->setMaterial(
			Stem::Inner, parent->getMaterial(Stem::Inner));
		pg::Path parentPath = parent->getPath();
		pathDivisions = parentPath.getResolution();
	} else
		this->stem = plant->createRoot();

	pg::Path path = this->stem->getPath();
	pg::Spline spline = path.getSpline();
	std::vector<pg::Vec3> controls;
	controls.push_back(pg::getZeroVec3());
	/* Adjust the second point at a latter time. */
	controls.push_back(pg::getZeroVec3());
	spline.setControls(controls);
	spline.setDegree(1);
	path.setSpline(spline);
	path.setResolution(pathDivisions);
	path.setRadius(pg::getDefaultCurve(0));
	this->stem->setPath(path);
	this->stem->setPosition(0.0f);

	this->selection->clear();
	this->selection->addStem(this->stem);
	this->selection->selectLastPoints();
}

void AddStem::setRadius()
{
	pg::Stem *parent = this->stem->getParent();
	if (parent) {
		pg::Path path = this->stem->getPath();
		pg::Path parentPath = parent->getPath();
		float t = this->stem->getPosition();
		float radius = parentPath.getIntermediateRadius(t);
		radius /= this->stem->getSwelling().x + 0.1;
		path.setMaxRadius(radius);
		if (parentPath.getMinRadius() > radius)
			path.setMinRadius(radius);
		else
			path.setMinRadius(parentPath.getMinRadius());
		this->stem->setPath(path);
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
	if (!this->moveStem.isDone()) {
		setRadius();
		return this->moveStem.onMousePress(event);
	} if (this->moveStem.isDone()) {
		bool update = this->movePath.onMouseRelease(event);
		this->done = this->movePath.isDone();
		if (this->done)
			this->stemCopy = *this->stem;
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
	this->prevSelection = *this->selection;
	create();
	this->moveStem.execute();
}

void AddStem::undo()
{
	pg::Plant *plant = this->selection->getPlant();
	plant->deleteStem(this->stem);
	*this->selection = this->prevSelection;
}

void AddStem::redo()
{
	pg::Plant *plant = this->selection->getPlant();
	this->stem = plant->addStem(this->stemCopy.getParent());
	*this->stem = this->stemCopy;
	this->selection->clear();
	this->selection->addStem(this->stem);
	this->selection->selectLastPoints();
}

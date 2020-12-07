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

using pg::Stem;
using pg::Plant;

AddStem::AddStem(
	Selection *selection, TranslationAxes *axes, const Camera *camera,
	int x, int y) :
	selection(selection),
	prevSelection(*selection),
	moveStem(selection, camera, x, y, true),
	movePath(selection, axes, camera)
{

}

void AddStem::create()
{
	Plant *plant = this->selection->getPlant();
	int pathDivisions = 1;

	auto instances = this->selection->getStemInstances();
	if (instances.size() == 1) {
		this->parent = (*instances.begin()).first;
		this->addition = plant->addStem(this->parent);
		this->addition->setSectionDivisions(
			this->parent->getSectionDivisions());
		this->addition->setMaterial(Stem::Outer,
			this->parent->getMaterial(Stem::Outer));
		this->addition->setMaterial(Stem::Inner,
			this->parent->getMaterial(Stem::Inner));
		pathDivisions = this->parent->getPath().getDivisions();
	} else if (instances.empty()) {
		this->addition = plant->createRoot();
		this->addition = this->addition;
		this->parent = nullptr;
	}

	pg::Path path = this->addition->getPath();
	pg::Spline spline = path.getSpline();
	std::vector<pg::Vec3> controls;
	controls.push_back(pg::Vec3(0.0f, 0.0f, 0.0f));
	controls.push_back(pg::Vec3(0.0f, 0.0f, 0.0f));
	spline.setControls(controls);
	spline.setDegree(1);
	path.setSpline(spline);
	path.setDivisions(pathDivisions);
	this->addition->setPath(path);
	this->addition->setDistance(0.0f);
	this->addition->setCustom(true);

	this->selection->clear();
	this->selection->addStem(this->addition);
	this->selection->selectLastPoints();
}

void AddStem::setRadius()
{
	if (this->parent) {
		Plant *plant = this->selection->getPlant();
		float t = this->addition->getDistance();
		float radius = plant->getIntermediateRadius(this->parent, t);
		radius /= this->addition->getSwelling().x + 0.1;
		this->addition->setMaxRadius(radius);
		if (this->parent->getMinRadius() > radius)
			this->addition->setMinRadius(radius);
		else {
			float minRadius = this->parent->getMinRadius();
			this->addition->setMinRadius(minRadius);
		}
	} else {
		this->addition->setMaxRadius(0.2f);
		this->addition->setSwelling(pg::Vec2(1.2f, 1.2f));
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
	extraction = plant->extractStem(this->addition);
	*this->selection = this->prevSelection;
}

void AddStem::redo()
{
	pg::Plant *plant = this->selection->getPlant();
	plant->reinsertStem(extraction);
	this->selection->clear();
	this->selection->addStem(this->addition);
	this->selection->selectLastPoints();
}

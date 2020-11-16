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
	prevSelection(*selection),
	moveStem(selection, camera, x, y, true),
	movePath(selection, axes, camera)
{
	this->selection = selection;
}

void AddStem::create()
{
	Plant *plant = this->selection->getPlant();
	Stem *stem = nullptr;
	int pathDivisions = 1;
	auto instances = this->selection->getStemInstances();
	if (instances.size() == 1) {
		extraction.parent = (*instances.begin()).first;
		stem = plant->addStem(extraction.parent);
		extraction.address = stem;
		stem->setSectionDivisions(
			extraction.parent->getSectionDivisions());
		stem->setMaterial(Stem::Outer,
			extraction.parent->getMaterial(Stem::Outer));
		stem->setMaterial(Stem::Inner,
			extraction.parent->getMaterial(Stem::Inner));
		pg::Path parentPath = extraction.parent->getPath();
		pathDivisions = parentPath.getDivisions();
	} else if (instances.empty()) {
		stem = plant->createRoot();
		extraction.address = stem;
		extraction.parent = nullptr;
	}

	pg::Path path = stem->getPath();
	pg::Spline spline = path.getSpline();
	std::vector<pg::Vec3> controls;
	controls.push_back(pg::Vec3(0.0f, 0.0f, 0.0f));
	/* Adjust the second point at a latter time. */
	controls.push_back(pg::Vec3(0.0f, 0.0f, 0.0f));
	spline.setControls(controls);
	spline.setDegree(1);
	path.setSpline(spline);
	path.setDivisions(pathDivisions);
	stem->setPath(path);
	stem->setDistance(0.0f);

	this->selection->clear();
	this->selection->addStem(stem);
	this->selection->selectLastPoints();
}

void AddStem::setRadius()
{
	Stem *stem = extraction.address;
	if (extraction.parent) {
		Plant *plant = this->selection->getPlant();
		Stem *parent = extraction.parent;
		float t = stem->getDistance();
		float radius = plant->getIntermediateRadius(parent, t);
		radius /= stem->getSwelling().x + 0.1;
		stem->setMaxRadius(radius);
		if (parent->getMinRadius() > radius)
			stem->setMinRadius(radius);
		else
			stem->setMinRadius(parent->getMinRadius());
	} else {
		stem->setMaxRadius(0.2f);
		stem->setSwelling(pg::Vec2(1.2f, 1.2f));
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
	extraction = plant->extractStem(extraction.address);
	*this->selection = this->prevSelection;
}

void AddStem::redo()
{
	pg::Plant *plant = this->selection->getPlant();
	plant->reinsertStem(extraction);
	this->selection->clear();
	this->selection->addStem(extraction.address);
	this->selection->selectLastPoints();
}

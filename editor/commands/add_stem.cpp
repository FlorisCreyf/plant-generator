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
	this->undone = false;
}

AddStem::~AddStem()
{
	if (!this->undone)
		delete this->stem;
}

void AddStem::create()
{
	auto instances = selection->getStemInstances();
	if (instances.size() == 1) {
		Stem *parent = (*instances.begin()).first;
		Plant *plant = selection->getPlant();
		this->stem = plant->addStem(parent);

		this->stem->setResolution(parent->getResolution());
		this->stem->setMaterial(
			Stem::Outer, parent->getMaterial(Stem::Outer));
		this->stem->setMaterial(
			Stem::Inner, parent->getMaterial(Stem::Inner));

		pg::Path parentPath = parent->getPath();
		pg::Path path = this->stem->getPath();
		pg::Spline spline = path.getSpline();
		std::vector<pg::Vec3> controls;
		controls.push_back(pg::getZeroVec3());
		/* Adjust the second point at a latter time. */
		controls.push_back(pg::getZeroVec3());
		spline.setControls(controls);
		spline.setDegree(1);
		path.setSpline(spline);
		path.setResolution(parentPath.getResolution());
		path.setRadius(pg::getDefaultCurve(0));
		this->stem->setPath(path);
		this->stem->setPosition(0.0f);

		selection->clear();
		selection->addStem(this->stem);
		selection->selectLastPoints();
	}
}

void AddStem::setRadius()
{
	pg::Stem *parent = this->stem->getParent();
	pg::Path path = this->stem->getPath();
	pg::Path parentPath = parent->getPath();
	float t = this->stem->getPosition();
	float radius = parentPath.getIntermediateRadius(t);
	radius /= stem->getSwelling().x + 0.1;
	path.setMaxRadius(radius);
	if (parentPath.getMinRadius() > radius)
		path.setMinRadius(radius);
	else
		path.setMinRadius(parentPath.getMinRadius());
	this->stem->setPath(path);
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

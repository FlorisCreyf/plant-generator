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

#include "rotate_stem.h"
#include <complex>

using pg::Vec3;
using pg::Vec4;
using pg::Quat;

RotateStem::RotateStem(
	Selection *selection, RotationAxes *axes,
	const Camera *camera, float x, float y)
{
	this->selection = selection;
	this->axes = axes;
	this->camera = camera;

	axes->setPosition(selection->getAveragePositionFP());
	axes->selectCenter();
	this->axis = axes->getSelection();

	this->firstDirection = Vec3(0.0f, 0.0f, 0.0f);
	pg::Ray ray = camera->getRay(x, y);
	Vec3 normal = camera->getDirection();
	set(ray, normal);

	checkValidity();
}

void RotateStem::checkValidity()
{
	this->valid = true;
	auto instances = this->selection->getStemInstances();
	for (auto instance : instances) {
		pg::Stem *stem1 = instance.first;
		for (auto instance : instances) {
			pg::Stem *stem2 = instance.first;
			if (stem1->isDescendantOf(stem2)) {
				this->valid = false;
				return;
			}
		}
	}
}

bool RotateStem::isValid()
{
	return this->valid;
}

void RotateStem::set(pg::Ray ray, Vec3 planeNormal)
{
	planeNormal = pg::normalize(planeNormal);
	pg::Plane plane = {this->axes->getPosition(), planeNormal};
	float distance = pg::intersectsPlane(ray, plane);
	this->lastDirection = this->direction;
	this->direction = distance * ray.direction + ray.origin;
	this->direction -= this->axes->getPosition();
	this->direction = pg::normalize(this->direction);
	this->ray = ray;
	this->planeNormal = planeNormal;
	if (pg::isZero(this->firstDirection))
		this->firstDirection = this->direction;
}

Vec3 RotateStem::getDirection()
{
	return direction;
}

Quat RotateStem::getTransformation(Quat q)
{
	Vec3 normal;
	if (this->axis == Axes::XAxis) {
		normal = Vec3(1.0f, 0.0f, 0.0f);
		Vec3 v = pg::rotate(q, normal);
		if (pg::dot(v, this->planeNormal) < 0)
			normal.x *= -1.0f;
	} else if (this->axis == Axes::YAxis) {
		normal = Vec3(0.0f, 1.0f, 0.0f);
		Vec3 v = pg::rotate(q, normal);
		if (pg::dot(v, this->planeNormal) < 0)
			normal.y *= -1.0f;
	} else if (this->axis == Axes::ZAxis) {
		normal = Vec3(0.0f, 0.0f, 1.0f);
		Vec3 v = pg::rotate(q, normal);
		if (pg::dot(v, this->planeNormal) < 0)
			normal.z *= -1.0f;
	} else
		normal = this->planeNormal;

	Quat t = pg::rotateIntoVecQ(this->planeNormal, normal);
	Vec3 a = pg::rotate(t, this->direction);
	Vec3 b = pg::rotate(t, this->lastDirection);
	return pg::rotateIntoVecQ(b, a);
}

void RotateStem::rotateChild(pg::Stem *stem, Quat t, float distance)
{
	while (stem) {
		if (stem->getDistance() >= distance) {
			pg::Path path = stem->getPath();
			pg::Spline spline = path.getSpline();
			std::vector<Vec3> controls = spline.getControls();
			for (size_t i = 0; i < controls.size(); i++) {
				Quat a = pg::toQuat(controls[i], 0.0f);
				controls[i] = pg::toVec3(t * a * conjugate(t));
			}
			spline.setControls(controls);
			path.setSpline(spline);
			stem->setPath(path);
			rotateChild(stem->getChild(), t, 0.0f);
		}
		rotateChildLeaves(stem, t, distance);
		stem = stem->getSibling();
	}
}

void RotateStem::rotateChildLeaves(pg::Stem *stem, Quat t, float distance)
{
	for (size_t i = 0; i < stem->getLeafCount(); i++) {
		pg::Leaf *leaf = stem->getLeaf(i);
		if (leaf->getPosition() >= distance)
			leaf->setRotation(t * leaf->getRotation());
	}
}

/** Rotate selected stems. */
void RotateStem::rotateStems()
{
	Quat q(0.0f, 0.0f, 0.0f, 1.0f);
	Quat t = getTransformation(q);
	auto stemInstances = this->selection->getStemInstances();
	for (auto &instance : stemInstances) {
		pg::Stem *stem = instance.first;
		PointSelection &ps = instance.second;
		int point = ps.hasPoints() ? *ps.getPoints().begin() : 0;
		pg::Path path = stem->getPath();
		pg::Spline spline = path.getSpline();
		auto controls = spline.getControls();

		if (spline.getDegree() == 3 && point % 3 == 0 && point > 0) {
			size_t i = point - 1;
			Vec3 p = controls[i] - controls[point];
			Quat q = pg::toQuat(p, 0.0f);
			controls[i] = pg::toVec3(t * q * pg::conjugate(t));
			controls[i] += controls[point];
		}

		for (size_t i = point + 1; i < controls.size(); i++) {
			Vec3 p = controls[i] - controls[point];
			Quat q = pg::toQuat(p, 0.0f);
			controls[i] = pg::toVec3(t * q * pg::conjugate(t));
			controls[i] += controls[point];
		}

		size_t pathIndex = path.toPathIndex(point);
		rotateChildLeaves(stem, t, path.getDistance(pathIndex));
		rotateChild(stem->getChild(), t, path.getDistance(pathIndex));

		spline.setControls(controls);
		path.setSpline(spline);
		stem->setPath(path);
	}
}

/** Rotate selected leaves. */
void RotateStem::rotateLeaves()
{
	auto leafInstances = this->selection->getLeafInstances();
	for (auto &instance : leafInstances) {
		pg::Stem *stem = instance.first;
		for (auto id : instance.second) {
			pg::Leaf *leaf = stem->getLeaf(id);
			Quat t = getTransformation(leaf->getRotation());
			leaf->setRotation(t * leaf->getRotation());
		}
	}
}

void RotateStem::execute()
{
	if (this->valid) {
		rotateStems();
		rotateLeaves();
	}
}

void RotateStem::undo()
{
	if (this->valid) {
		Vec3 d = this->firstDirection;
		this->firstDirection = this->lastDirection = this->direction;
		this->direction = d;
		execute();
		d = this->firstDirection;
		this->firstDirection = this->lastDirection = this->direction;
		this->direction = d;
	}
}

void RotateStem::resetRotation()
{
	this->lastDirection = this->direction;
	this->direction = this->firstDirection;
	execute();
	this->lastDirection = this->firstDirection;
	this->direction = this->firstDirection;
}

bool RotateStem::onMouseMove(QMouseEvent *event)
{
	QPoint point = event->pos();
	float x = point.x();
	float y = point.y();

	if (this->axes->getSelection() != this->axis) {
		resetRotation();
		this->axis = this->axes->getSelection();
	}

	set(this->camera->getRay(x, y), this->camera->getDirection());
	execute();

	return true;
}

bool RotateStem::onMousePress(QMouseEvent *)
{
	this->done = true;
	return false;
}

bool RotateStem::onKeyPress(QKeyEvent *event)
{
	char key = event->key();
	if (key == Qt::Key_C)
		this->axes->selectAxis(Axes::Center);
	else if (key == Qt::Key_X)
		this->axes->selectAxis(Axes::XAxis);
	else if (key == Qt::Key_Y)
		this->axes->selectAxis(Axes::YAxis);
	else if (key == Qt::Key_Z)
		this->axes->selectAxis(Axes::ZAxis);
	return false;
}

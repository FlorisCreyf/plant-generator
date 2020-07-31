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
	axis = axes->getSelection();

	firstDirection = Vec3(0.0f, 0.0f, 0.0f);
	pg::Ray ray = camera->getRay(x, y);
	Vec3 normal = camera->getDirection();
	set(ray, normal);

	checkValidity();
}

void RotateStem::checkValidity()
{
	valid = true;
	auto instances = selection->getStemInstances();
	for (auto instance : instances) {
		pg::Stem *stem1 = instance.first;
		for (auto instance : instances) {
			pg::Stem *stem2 = instance.first;
			if (stem1->isDescendantOf(stem2)) {
				valid = false;
				return;
			}
		}
	}
}

bool RotateStem::isValid()
{
	return valid;
}

void RotateStem::set(pg::Ray ray, Vec3 planeNormal)
{
	planeNormal = pg::normalize(planeNormal);
	pg::Plane plane = {axes->getPosition(), planeNormal};
	float distance = pg::intersectsPlane(ray, plane);
	lastDirection = direction;
	direction = distance * ray.direction + ray.origin - axes->getPosition();
	direction = pg::normalize(direction);
	this->ray = ray;
	this->planeNormal = planeNormal;
	if (pg::isZero(firstDirection))
		firstDirection = direction;
}

Vec3 RotateStem::getDirection()
{
	return direction;
}

Quat RotateStem::getTransformation(Quat q)
{
	Vec3 normal;

	if (this->axis == Axes::XAxis) {
		normal.x = 1.0f;
		normal.y = 0.0f;
		normal.z = 0.0f;
		Quat r = q * pg::toQuat(normal, 0.0f) * pg::conjugate(q);
		Vec3 v = pg::toVec3(r);
		if (pg::dot(v, this->planeNormal) < 0)
			normal.x *= -1.0f;
	} else if (this->axis == Axes::YAxis) {
		normal.x = 0.0f;
		normal.y = 1.0f;
		normal.z = 0.0f;
		Quat r = q * pg::toQuat(normal, 0.0f) * pg::conjugate(q);
		Vec3 v = pg::toVec3(r);
		if (pg::dot(v, this->planeNormal) < 0)
			normal.y *= -1.0f;
	} else if (this->axis == Axes::ZAxis) {
		normal.x = 0.0f;
		normal.y = 0.0f;
		normal.z = 1.0f;
		Quat r = q * pg::toQuat(normal, 0.0f) * pg::conjugate(q);
		Vec3 v = pg::toVec3(r);
		if (pg::dot(v, this->planeNormal) < 0)
			normal.z *= -1.0f;
	} else
		normal = this->planeNormal;

	Quat t = pg::rotateIntoVecQ(this->planeNormal, normal);
	Quat a = t * pg::toQuat(this->direction, 0.0f) * pg::conjugate(t);
	Quat b = t * pg::toQuat(this->lastDirection, 0.0f) * pg::conjugate(t);
	return pg::rotateIntoVecQ(pg::toVec3(b), pg::toVec3(a));
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

		stem = stem->getSibling();
	}
}

void RotateStem::rotateStems()
{
	Quat q(0.0f, 0.0f, 0.0f, 1.0f);
	Quat t = getTransformation(q);
	auto stemInstances = selection->getStemInstances();
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
		rotateChild(stem->getChild(), t, path.getDistance(pathIndex));

		spline.setControls(controls);
		path.setSpline(spline);
		stem->setPath(path);
	}
}

void RotateStem::rotateLeaves()
{
	auto leafInstances = selection->getLeafInstances();
	for (auto &instance : leafInstances) {
		pg::Stem *stem = instance.first;
		pg::Path path = stem->getPath();
		for (auto id : instance.second) {
			pg::Leaf *leaf = stem->getLeaf(id);

			/* Create a rotation 'q' that transforms the global
			axes into the axes of each leaf. This is needed to
			determine if rotations move in the same direction as
			the cursor. */
			float position = leaf->getPosition();
			Vec3 d = path.getIntermediateDirection(position);
			Quat q = leaf->getDefaultOrientation(d);
			Quat t = getTransformation(q);
			leaf->setRotation(t * leaf->getRotation());
		}
	}
}

void RotateStem::execute()
{
	if (valid) {
		rotateStems();
		rotateLeaves();
	}
}

void RotateStem::undo()
{
	if (valid) {
		Vec3 d = firstDirection;
		firstDirection = lastDirection = direction;
		direction = d;
		execute();
		d = firstDirection;
		firstDirection = lastDirection = direction;
		direction = d;
	}
}

void RotateStem::resetRotation()
{
	lastDirection = direction;
	direction = firstDirection;
	execute();
	lastDirection = firstDirection;
	direction = firstDirection;
}

bool RotateStem::onMouseMove(QMouseEvent *event)
{
	QPoint point = event->pos();
	float x = point.x();
	float y = point.y();

	if (axes->getSelection() != axis) {
		resetRotation();
		axis = axes->getSelection();
	}

	set(this->camera->getRay(x, y), this->camera->getDirection());
	execute();

	return true;
}

bool RotateStem::onMousePress(QMouseEvent *)
{
	done = true;
	return false;
}

bool RotateStem::onKeyPress(QKeyEvent *event)
{
	char key = event->key();
	if (key == Qt::Key_C)
		axes->selectAxis(Axes::Center);
	else if (key == Qt::Key_X)
		axes->selectAxis(Axes::XAxis);
	else if (key == Qt::Key_Y)
		axes->selectAxis(Axes::YAxis);
	else if (key == Qt::Key_Z)
		axes->selectAxis(Axes::ZAxis);
	return false;
}

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

#include "rotate_stem.h"
#include <complex>

RotateStem::RotateStem(Selection *selection, RotationAxes *axes)
{
	this->selection = selection;
	this->axes = axes;
	firstDirection = pg::getZeroVec3();
	axes->setPosition(selection->getAveragePositionFP());
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

void RotateStem::set(pg::Ray ray, pg::Vec3 planeNormal)
{
	updatedAxis = axes->getSelection();
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

pg::Vec3 RotateStem::getDirection()
{
	return direction;
}

pg::Quat RotateStem::getTransformation(pg::Quat q)
{
	pg::Vec3 v;
	pg::Vec3 normal;
	switch (axis) {
	case Axes::XAxis:
		normal.x = 1.0f;
		normal.y = 0.0f;
		normal.z = 0.0f;
		v = pg::toVec3(q * pg::toVec4(normal, 0.0f) * pg::conjugate(q));
		if (pg::dot(v, planeNormal) < 0)
			normal.x *= -1.0f;
		break;
	case Axes::YAxis:
		normal.x = 0.0f;
		normal.y = 1.0f;
		normal.z = 0.0f;
		v = pg::toVec3(q * pg::toVec4(normal, 0.0f) * pg::conjugate(q));
		if (pg::dot(v, planeNormal) < 0)
			normal.y *= -1.0f;
		break;
	case Axes::ZAxis:
		normal.x = 0.0f;
		normal.y = 0.0f;
		normal.z = 1.0f;
		v = pg::toVec3(q * pg::toVec4(normal, 0.0f) * pg::conjugate(q));
		if (pg::dot(v, planeNormal) < 0)
			normal.z *= -1.0f;
		break;
	default:
		normal = planeNormal;
		break;
	}

	pg::Quat t = pg::rotateIntoVecQ(planeNormal, normal);
	pg::Quat a = t * pg::toVec4(direction, 0.0f) * pg::conjugate(t);
	pg::Quat b = t * pg::toVec4(lastDirection, 0.0f) * pg::conjugate(t);
	return pg::rotateIntoVecQ(pg::toVec3(b), pg::toVec3(a));
}

void RotateStem::rotateChild(pg::Stem *stem, pg::Quat t, float distance)
{
	while (stem) {
		if (stem->getPosition() >= distance) {
			pg::VolumetricPath path = stem->getPath();
			pg::Spline spline = path.getSpline();
			std::vector<pg::Vec3> controls = spline.getControls();
			for (size_t i = 0; i < controls.size(); i++) {
				pg::Vec4 a = pg::toVec4(controls[i], 0.0f);
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

void RotateStem::rotate()
{
	pg::Quat q = {0.0f, 0.0f, 0.0f, 1.0f};
	pg::Quat t = getTransformation(q);
	auto stemInstances = selection->getStemInstances();
	for (auto &instance : stemInstances) {
		pg::Stem *stem = instance.first;
		PointSelection &ps = instance.second;
		int point = ps.hasPoints() ? *ps.getPoints().begin() : 0;
		pg::VolumetricPath path = stem->getPath();
		pg::Spline spline = path.getSpline();
		auto controls = spline.getControls();

		if (spline.getDegree() == 3 && point % 3 == 0 && point > 0) {
			size_t i = point - 1;
			pg::Vec3 p = controls[i] - controls[point];
			controls[i] = toVec3(t * toVec4(p, 0.0f) * conjugate(t));
			controls[i] += controls[point];
		}

		for (size_t i = point + 1; i < controls.size(); i++) {
			pg::Vec3 p = controls[i] - controls[point];
			controls[i] = toVec3(t * toVec4(p, 0.0f) * conjugate(t));
			controls[i] += controls[point];
		}

		rotateChild(stem->getChild(), t, path.getDistance(point));

		spline.setControls(controls);
		path.setSpline(spline);
		stem->setPath(path);
	}

	auto leafInstances = selection->getLeafInstances();
	for (auto &instance : leafInstances) {
		pg::Stem *stem = instance.first;
		pg::VolumetricPath path = stem->getPath();
		for (auto id : instance.second) {
			pg::Leaf *leaf = stem->getLeaf(id);

			/* Create a rotation 'q' that transforms the global axes
			 * into the axes of each leaf. This is needed to
			 * determine if rotations move in the same direction
			 * as the cursor. */
			float position = leaf->getPosition();
			pg::Vec3 d = path.getIntermediateDirection(position);
			pg::Quat q = leaf->getDefaultOrientation(d);
			t = getTransformation(q);
			leaf->setRotation(t * leaf->getRotation());
		}
	}
}

void RotateStem::execute()
{
	if (valid) {
		pg::Vec3 tempDirection = direction;
		if (updatedAxis != axis) {
			direction = firstDirection;
		}

		rotate();

		if (updatedAxis != axis) {
			axis = axes->getSelection();
			lastDirection = firstDirection;
			direction = tempDirection;
			rotate();
		}
	}
}

void RotateStem::undo()
{
	if (valid) {
		pg::Vec3 d = firstDirection;
		firstDirection = lastDirection = direction;
		direction = d;
		execute();

		/* Set variables for redo. */
		d = firstDirection;
		firstDirection = lastDirection = direction;
		direction = d;
	}
}

RotateStem *RotateStem::clone()
{
	return new RotateStem(*this);
}

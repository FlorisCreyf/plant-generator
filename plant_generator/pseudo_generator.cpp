/* Copyright 2017 Floris Creyf
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "plant.h"
#include "pseudo_generator.h"
#include "math.h"
#include <cstdlib>
#include <cmath>

#define PI 3.14159265359f

using namespace pg;

PseudoGenerator::PseudoGenerator(Plant *plant)
{
	this->plant = plant;
	this->randomGenerator.seed(this->dvn.seed);
}

Derivation PseudoGenerator::getDerivation() const
{
	return this->dvn;
}

void PseudoGenerator::setDerivation(Derivation dvn)
{
	this->dvn = dvn;
}

void PseudoGenerator::reset()
{
	this->randomGenerator.seed(this->dvn.seed);
}

void PseudoGenerator::grow()
{
	Stem *root = this->plant->createRoot();
	root->setDerivation(this->dvn);
	root->setPosition(0.0f);
	root->setMaxRadius(0.2f);
	root->setMinRadius(0.01f);
	setPath(root, Vec3(0.0f, 1.0f, 0.0f));
	reset();
	addLateralStems(root);
}

void PseudoGenerator::grow(Stem *stem)
{
	this->dvn = stem->getDerivation();
	reset();
	addLateralStems(stem);
}

Vec3 PseudoGenerator::getStemDirection(Stem *stem)
{
	Path path = stem->getParent()->getPath();
	float ratio = stem->getPosition() / path.getLength();
	Vec3 direction = path.getIntermediateDirection(stem->getPosition());
	std::uniform_real_distribution<float> dis(0.0f, 2.0*PI);
	float angleX = PI*(0.1f + 0.4f*(1.0f-ratio));
	float angleY = dis(this->randomGenerator);
	Mat4 rot = pg::rotateXY(angleX, angleY);
	Mat4 tran = pg::rotateIntoVec(Vec3(0.0f, 1.0f, 0.0f), direction);
	tran = tran * rot;
	return tran.apply(Vec3(0.0f, 1.0f, 0.0f), 1.0f);
}

void PseudoGenerator::addLateralStems(Stem *parent)
{
	if (this->dvn.stemDensity == 0.0f)
		return;

	float length = parent->getPath().getLength();
	float distance = 1.0f / this->dvn.stemDensity;
	float position = this->dvn.stemStart;

	while (position < length) {
		float t = position / length;
		float percentage = dvn.stemDensityCurve.getPoint(t).z;
		if (percentage == 0.0f)
			break;
		addLateralStem(parent, position);
		position += distance * (1.0f/percentage);
	}
}

void PseudoGenerator::addLateralStem(Stem *parent, float position)
{
	Vec2 swelling(1.5f, 3.0f);
	float radius = getRadius(parent, swelling.x, position);
	if (radius < this->dvn.radiusThreshold)
		return;

	Stem *stem = plant->addStem(parent);
	stem->setMaxRadius(radius);
	stem->setMinRadius(getMinRadius(radius));
	stem->setSwelling(swelling);
	stem->setPosition(position);
	stem->setResolution(5);
	setPath(stem, getStemDirection(stem));
	addLeaves(stem);

	if (stem->getDepth() < this->dvn.depth)
		addLateralStems(stem);
}

float PseudoGenerator::getRadius(Stem *parent, float swelling, float position)
{
	float r = this->plant->getIntermediateRadius(parent, position);
	return r / (swelling + 0.1f);
}

float PseudoGenerator::getMinRadius(float radius)
{
	float minRadius = radius / 5.0f;
	if (minRadius < 0.001f)
		minRadius = 0.001f;
	else if (minRadius > 0.01f)
		minRadius = 0.01f;
	return minRadius;
}

void PseudoGenerator::setPath(Stem *stem, Vec3 direction)
{
	int divisions = 1;
	float radius = stem->getMaxRadius();
	float length = radius * this->dvn.lengthFactor;
	int points = static_cast<int>(length / 2.0f) + 1;
	float increment = length / points;

	Path path;
	path.setResolution(divisions);
	std::vector<Vec3> controls;
	std::uniform_real_distribution<float> dis(-0.05f, 0.05f);

	Vec3 control(0.0f, 0.0f, 0.0f);
	controls.push_back(control);
	control += radius * 5.0f * direction;
	controls.push_back(control);

	for (int i = 0; i < points; i++) {
		control = control + increment * direction;
		direction.x += dis(this->randomGenerator);
		direction.y += dis(this->randomGenerator);
		direction.z += dis(this->randomGenerator);
		direction = normalize(direction);
		controls.push_back(control);
	}

	Spline spline;
	spline.setDegree(1);
	spline.setControls(controls);
	path.setSpline(spline);
	stem->setPath(path);
}

void PseudoGenerator::addLeaves(Stem *stem)
{
	if (this->dvn.leafDensity <= 0.0f)
		return;

	float length = stem->getPath().getLength();
	float distance = 1.0f / this->dvn.leafDensity;
	float position = this->dvn.leafStart;
	Quat rotation(0.0f, 0.0f, 0.0f, 1.0f);

	if (this->dvn.arrangement == Derivation::Alternate) {
		while (position < length) {
			float t = position / length;
			float percentage = dvn.leafDensityCurve.getPoint(t).z;
			if (percentage == 0.0f)
				break;

			addLeaf(stem, position, rotation);
			rotation = alternate(rotation);
			position += distance * (1.0f/percentage);
		}
	} else if (this->dvn.arrangement == Derivation::Opposite) {
		while (position < length) {
			float t = position / length;
			float percentage = dvn.leafDensityCurve.getPoint(t).z;
			if (percentage == 0.0f)
				break;

			addLeaf(stem, position, rotation);
			rotation = alternate(rotation);
			addLeaf(stem, position, rotation);
			rotation = alternate(rotation);
			position += distance * (1.0f/percentage);
		}
	} else if (this->dvn.arrangement == Derivation::Whorled) {
		Vec3 axis = Vec3(1.0f, 0.0f, 0.0f);
		Quat increment = fromAxisAngle(axis, 2.0f*PI/3.0f);
		while (position < length) {
			float t = position / length;
			float percentage = dvn.leafDensityCurve.getPoint(t).z;
			if (percentage == 0.0f)
				break;

			rotation = Quat(1.0f, 0.0f, 0.0f, 0.0f);
			addLeaf(stem, position, rotation);
			rotation *= increment;
			addLeaf(stem, position, rotation);
			rotation *= increment;
			addLeaf(stem, position, rotation);
			position += distance * (1.0f/percentage);
		}
	}
}

void PseudoGenerator::addLeaf(Stem *stem, float position, Quat rotation)
{
	Leaf leaf;
	leaf.setPosition(position);
	leaf.setScale(Vec3(2.0f, 2.0f, 2.0f));
	leaf.setRotation(rotation);
	stem->addLeaf(leaf);
}

Quat PseudoGenerator::alternate(Quat prevRotation)
{
	Quat rotation(0.0f, 0.0f, 0.0f, 0.0f);
	if (prevRotation.w == 1.0f)
		rotation.y = 1.0f;
	else
		rotation.w = 1.0f;
	return rotation;
}

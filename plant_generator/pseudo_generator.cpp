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
#include "patterns.h"
#include "math.h"
#include <cstdlib>
#include <cmath>

#define PI 3.14159265359f

using namespace pg;

PseudoGenerator::PseudoGenerator(Plant *plant)
{
	this->plant = plant;
	this->dvn.seed = 0;
	this->randomGenerator.seed(this->dvn.seed);
	this->dvn.depth = 1;
	this->dvn.stemDensity = 0.0f;
	this->dvn.leafDensity = 0.0f;
	this->dvn.stemStart = 0.0f;
	this->dvn.leafStart = 0.0f;
	this->dvn.requiredLength = 0.0f;
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
	setPath(root, nullptr, Vec3(0.0f, 1.0f, 0.0f), 0.0f);
	reset();
	addLateralStems(root);
}

void PseudoGenerator::grow(Stem *stem)
{
	stem->setDerivation(this->dvn);
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
		if (!growLateralStem(parent, position))
			break;
		position += distance;
	}
}

bool PseudoGenerator::growLateralStem(Stem *parent, float position)
{
	Stem *stem = plant->addStem(parent);
	stem->setPosition(position);

	if (parent->getResolution() - 4 < 3)
		stem->setResolution(3);
	else
		stem->setResolution(parent->getResolution() - 2);

	if (!setPath(stem, parent, getStemDirection(stem), position)) {
		this->plant->deleteStem(stem);
		return false;
	}

	if (this->dvn.leafDensity > 0.0f) {
		float length = stem->getPath().getLength();
		float distance = 1.0f / this->dvn.leafDensity;
		float position = this->dvn.leafStart;
		Quat rotation(0.0f, 0.0f, 0.0f, 1.0f);
		while (position < length) {
			Leaf leaf;
			leaf.setPosition(position);
			leaf.setScale(Vec3(2.0f, 2.0f, 2.0f));
			alternateLeaf(&leaf, rotation);
			rotation = leaf.getRotation();
			stem->addLeaf(leaf);
			position += distance;
		}
	}

	if (stem->getDepth() < this->dvn.depth)
		addLateralStems(stem);

	return true;
}

bool PseudoGenerator::setPath(
	Stem *stem, Stem *parent, Vec3 direction, float position)
{
	float radius;
	float minRadius;
	float parentRadius;
	if (parent) {
		parentRadius = this->plant->getIntermediateRadius(
			parent, position);
		radius = parentRadius / (stem->getSwelling().x + 0.1f);
		minRadius = radius / 5.0f;
		if (minRadius < 0.001f)
			minRadius = 0.001f;
		else if (minRadius > 0.01f)
			minRadius = 0.01f;
	} else {
		radius = 0.2f;
		minRadius = 0.01f;
		parentRadius = 1.0f;
	}

	int divisions = 1;
	float length = radius * 40.0f;
	int points = static_cast<int>(length / 2.0f) + 1;
	float increment = length / points;
	if (length < this->dvn.requiredLength)
		return false;

	stem->setMinRadius(minRadius);
	stem->setMaxRadius(radius);

	Path path;
	path.setResolution(divisions);
	std::vector<Vec3> controls;
	std::uniform_real_distribution<float> dis(-0.05f, 0.05f);

	Vec3 control(0.0f, 0.0f, 0.0f);
	controls.push_back(control);
	control += parentRadius * 4.0f * direction;
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
	return true;
}

void PseudoGenerator::alternateLeaf(Leaf *leaf, Quat prevRotation)
{
	Quat rotation(0.0f, 0.0f, 0.0f, 0.0f);
	if (prevRotation.w == 1.0f)
		rotation.y = 1.0f;
	else
		rotation.w = 1.0f;
	leaf->setRotation(rotation);
}

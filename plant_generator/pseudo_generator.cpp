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

using pg::Stem;
using pg::Vec3;
using pg::Mat4;

pg::PseudoGenerator::PseudoGenerator(pg::Plant *plant)
{
	std::random_device rd;
	randomGenerator.seed(rd());
	this->plant = plant;
	this->maxStemDepth = 1;
	this->hasLeaves = true;
}

void pg::PseudoGenerator::grow()
{
	Stem *root = plant->createRoot();
	root->setPosition(0.0f);
	setPath(root, nullptr, {0.0f, 1.0f, 0.0f}, 0.0f);
	addLateralStems(root, 1.5f);
}

Vec3 pg::PseudoGenerator::getStemDirection(Stem *stem)
{
	Path path = stem->getParent()->getPath();
	float ratio = stem->getPosition() / path.getLength();
	Vec3 direction = path.getIntermediateDirection(stem->getPosition());
	std::uniform_real_distribution<float> dis(0.0f, 2.0*M_PI);
	float angleX = M_PI*(0.1f + 0.4f*(1.0f-ratio));
	float angleY = dis(this->randomGenerator);
	Mat4 rot = pg::rotateXY(angleX, angleY);
	Mat4 tran = pg::rotateIntoVec({0.0f, 1.0f, 0.0f}, direction);
	tran = tran * rot;
	return tran.apply({0.0f, 1.0f, 0.0f}, 1.0f);
}

void pg::PseudoGenerator::addLateralStems(Stem *parent, float position)
{
	float length = parent->getPath().getLength();
	float stemDensity = parent->getDepth() == 0 ? 1.0f : 1.5f;
	float distance = 1.0f / stemDensity;

	while (position < length) {
		Stem *stem = plant->addStem(parent);
		growLateralStem(stem, position);
		position += distance;
	}
}

void pg::PseudoGenerator::growLateralStem(Stem *stem, float position)
{
	Stem *parent = stem->getParent();
	stem->setPosition(position);

	if (parent->getResolution() - 4 < 3)
		stem->setResolution(3);
	else
		stem->setResolution(parent->getResolution() - 2);

	setPath(stem, parent, getStemDirection(stem), position);

	if (this->hasLeaves) {
		Leaf leaf;
		leaf.setPosition(1.5f);
		leaf.setScale({2.0f, 2.0f, 2.0f});
		stem->addLeaf(leaf);
	}

	if (stem->getDepth() < this->maxStemDepth)
		addLateralStems(stem, 0.5f);
}

void pg::PseudoGenerator::setPath(
	Stem *stem, Stem *parent, Vec3 direction, float position)
{
	std::vector<Vec3> controls;
	std::uniform_real_distribution<float> dis(-0.05f, 0.05f);

	float radius;
	float minRadius;
	if (parent) {
		float margin = 1.0f / stem->getResolution();
		radius = parent->getPath().getIntermediateRadius(position);
		radius /= stem->getSwelling().x + margin;
		minRadius = radius / 5.0f;
		if (minRadius < 0.001f)
			minRadius = 0.001f;
		else if (minRadius > 0.01f)
			minRadius = 0.01f;
	} else {
		radius = 0.2f;
		minRadius = 0.01f;
	}

	float length = radius * 10.0f;
	int divisions = stem->getDepth() == 0 ? 2 : 1;
	int points = stem->getParent() ? 4 : 5;

	Path path;
	path.setMinRadius(minRadius);
	path.setMaxRadius(radius);
	path.setRadius(getDefaultCurve(0));
	path.setResolution(divisions);

	Vec3 control = {0.0f, 0.0f, 0.0f};
	controls.push_back(control);
	control += length * 0.5f * direction;
	controls.push_back(control);

	for (int i = 0; i < points; i++) {
		control = control + length * direction;
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

void pg::PseudoGenerator::disableLeaves(bool disable)
{
	this->hasLeaves = !disable;
}

void pg::PseudoGenerator::setMaxDepth(int depth)
{
	this->maxStemDepth = depth;
}

int pg::PseudoGenerator::getMaxDepth()
{
	return maxStemDepth;
}

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
#include "generator.h"
#include "patterns.h"
#include "math.h"
#include <cstdlib>
#include <cmath>

using pg::Stem;
using pg::Vec3;
using pg::Mat4;

pg::Generator::Generator(pg::Plant *plant)
{
	std::random_device rd;
	randomGenerator.seed(rd());
	this->plant = plant;
}

Vec3 pg::Generator::getStemDirection(Stem *stem)
{
	Path path = stem->getParent()->getPath();
	Vec3 direction = path.getIntermediateDirection(stem->getPosition());
	float radius = stem->getPath().getMaxRadius();
	float parentRadius = stem->getParent()->getPath().getMaxRadius();
	float ratio = radius / parentRadius;
	std::uniform_real_distribution<float> dis(0.0f, M_PI * 0.4f);
	float angleX = dis(randomGenerator) * ratio;
	float angleY = dis(randomGenerator);
	Mat4 rot = pg::rotateXY(M_PI * 0.6f - angleX, angleY * M_PI * 2.0f);
	Mat4 tran = pg::rotateIntoVec({0.0f, 1.0f, 0.0f}, direction);
	tran = tran * rot;
	return pg::toVec3(tran * pg::toVec4({0.0f, 1.0f, 0.0f}, 1.0f));
}

void pg::Generator::getDichotomousDirections(Stem *parent, Vec3 directions[2])
{
	std::uniform_real_distribution<float> dis(0.0f, 1.0f);
	float angleX = dis(randomGenerator) * 0.5f;
	float angleY = dis(randomGenerator) + M_PI*0.25f;

	Path path = parent->getPath();
	Vec3 normal = path.getDirection(path.getResolution() - 1);
	Vec3 b = pg::cross(normal, {normal.x, 0.0f, normal.z});

	b = pg::rotateAroundAxis(b, normal, angleY);
	directions[0] = pg::rotateAroundAxis(normal, b, angleX);
	directions[1] = pg::rotateAroundAxis(normal, b, -angleX);
}

float pg::Generator::getRadius(Stem *stem)
{
	Stem *parent = stem->getParent();
	float radius;
	if (parent) {
		radius = stem->getParent()->getPath().getMaxRadius();
		radius *= 0.5f * std::pow(2.0f, -stem->getPosition()/5.0f);
	} else
		radius = 0.2f;
	return radius;
}

void pg::Generator::setPath(Stem *stem, Vec3 direction)
{
	std::vector<Vec3> controls;
	Vec3 control = {0.0f, 0.0f, 0.0f};
	Vec3 variance = {0.02f, -0.05f, 0.01f};
	float radius = getRadius(stem);
	float length = 15.0f * radius;
	int divisions = stem->getDepth() == 0 ? 2 : 1;
	int points = stem->getParent() ? 3 : 4;

	Path path;
	path.setMaxRadius(radius);
	path.setRadius(getDefaultCurve(0));
	path.setResolution(divisions);

	for (int i = 0; i < points; i++) {
		controls.push_back(control);
		control = control + length * direction;
		direction = normalize(direction + variance);
	}

	/* Thinner stems are more flexible and should bend more towards
	 * the light source. */
	controls.back().y += 0.1f;

	Spline spline;
	spline.setDegree(1);
	spline.setControls(controls);
	path.setSpline(spline);
	stem->setPath(path);
}

void pg::Generator::addLateralStems(Stem *parent, float position)
{
	float length = parent->getPath().getLength();
	float stemDensity = parent->getDepth() == 0 ? 1.0f : 0.0f;
	float distance = 1.0f / stemDensity;

	while (position < length) {
		Stem *stem = plant->addStem(parent);
		growLateralStem(stem, position);
		position += distance;
	}
}

void pg::Generator::growLateralStem(Stem *stem, float position)
{
	Stem *parent = stem->getParent();
	stem->setPosition(position);
	if (parent->getResolution() - 4 < 5)
		stem->setResolution(5);
	else
		stem->setResolution(parent->getResolution() - 4);
	setPath(stem, getStemDirection(stem));

	Leaf leaf;
	leaf.setPosition(1.5f);
	leaf.setScale((Vec3){2.0f, 2.0f, 2.0f});
	stem->addLeaf(leaf);
}

void pg::Generator::grow()
{
	plant->removeRoot();
	plant->setRoot(new Stem(nullptr));
	Stem *root = plant->getRoot();
	root->setPosition(0.0f);
	setPath(root, {0.0f, 1.0f, 0.0f});
	addLateralStems(root, 1.5f);
}

void pg::Generator::setMaxDepth(int depth)
{
	maxStemDepth = depth;
}

int pg::Generator::getMaxDepth()
{
	return maxStemDepth;
}

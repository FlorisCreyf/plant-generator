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
#include "pattern_generator.h"
#include <cstdlib>
#include <cmath>

using namespace pg;

const float pi = 3.14159265359f;

PatternGenerator::PatternGenerator(Plant *plant) : plant(plant)
{

}

ParameterTree PatternGenerator::getParameterTree() const
{
	return this->parameterTree;
}

void PatternGenerator::setParameterTree(ParameterTree parameterTree)
{
	this->parameterTree = parameterTree;
}

void PatternGenerator::reset()
{
	ParameterNode *root = this->parameterTree.getRoot();
	if (root) {
		this->mt.seed(root->getData().seed);
		this->mt.discard(100);
	}
}

void PatternGenerator::grow()
{
	Stem *stem = this->plant->createRoot();
	stem->setParameterTree(this->parameterTree);
	stem->setDistance(0.0f);
	stem->setMaxRadius(0.2f);
	stem->setMinRadius(0.01f);
	stem->setSwelling(Vec2(1.3f, 1.3f));
	const ParameterNode *root = this->parameterTree.getRoot();
	if (root) {
		reset();
		addStems(stem, Vec3(0.0f, 1.0f, 0.0f), 1.0f, root);
	}
}

void PatternGenerator::grow(Stem *stem)
{
	this->parameterTree = stem->getParameterTree();
	ParameterNode *node = this->parameterTree.getRoot();
	if (node) {
		reset();
		const Path &path = stem->getPath();
		addStems(stem, path.getDirection(0), 1.0f, node);
	}
}

float getBifurcationLength(Stem *stem)
{
	Stem *fork[2];
	stem->getFork(fork);
	if (fork[0] && fork[1]) {
		float l = stem->getPath().getLength();
		return l + getBifurcationLength(fork[1]);
	} else
		return stem->getPath().getLength();
}

float PatternGenerator::addStems(Stem *stem, Vec3 direction, float ratio,
	const ParameterNode *node)
{
	const StemData &data = node->getData();
	if (!stem->isCustom())
		ratio = setPath(stem, direction, ratio, data);
	float length = stem->getPath().getLength();

	bool customFork = false;
	if (stem->isCustom()) {
		Stem *fork[2];
		stem->getFork(fork);
		customFork = fork[0] && fork[1];
		if (customFork)
			length += getBifurcationLength(fork[1]);
	}

	if (!stem->isCustom() && ratio <= 0.0f)
		stem->setMinRadius(0.0f);
	else if (!customFork) {
		float l = 0.0f;
		for (int i = 0; i < 2; i++) {
			Stem *fork = plant->addStem(stem);
			fork->setMaxRadius(stem->getMinRadius());
			fork->setDistance(std::numeric_limits<float>::max());
			fork->setSectionDivisions(stem->getSectionDivisions());
			Vec3 direction = getForkDirection(fork, data, i);
			l = addStems(fork, direction, ratio, node);
		}
		length += l;
	}

	node = node->getChild();
	while (node) {
		addLateralStems(stem, length, node);
		addLeaves(stem, node->getData().leaf, length);
		node = node->getSibling();
	}
	return length;
}

void PatternGenerator::addLateralStems(Stem *parent, float length,
	const ParameterNode *node)
{
	StemData stemData = node->getData();
	if (stemData.density == 0.0f)
		return;

	float distance = 1.0f / stemData.density;
	float position = parent->getPath().getLength() - distance;
	float end = (length - stemData.distance);
	if (end <= distance)
		end = distance;

	for (int i = 0; position > end; i++) {
		float t = position / length;
		float r = stemData.densityCurve.getPoint(t).z;
		if (r == 0.0f)
			break;
		addLateralStem(parent, position, node, i);
		position -= distance * (1.0f/r);
	}
}

void PatternGenerator::addLateralStem(Stem *parent, float position,
	const ParameterNode *node, int index)
{
	StemData data = node->getData();
	Vec2 collar(1.5f, 3.0f);
	float radius = this->plant->getIntermediateRadius(parent, position);
	radius = radius / collar.x * data.scale;
	if (radius < data.radiusThreshold)
		return;

	Stem *stem = plant->addStem(parent);
	stem->setMaxRadius(radius);
	stem->setSwelling(collar);
	stem->setDistance(position);
	if (parent->getSectionDivisions() > 4)
		stem->setSectionDivisions(parent->getSectionDivisions()-2);
	else
		stem->setSectionDivisions(parent->getSectionDivisions());

	Vec3 direction = getStemDirection(stem, data, index);
	addStems(stem, direction, 1.0f, node);
}

Vec3 PatternGenerator::getForkDirection(Stem *stem, const StemData &data, int i)
{
	float minAngle = 0.1f;
	float maxAngle = data.forkAngle;
	if (data.forkAngle < minAngle)
		maxAngle = minAngle;

	std::uniform_real_distribution<float> dis(minAngle, maxAngle);
	const Path &path = stem->getParent()->getPath();
	Vec3 parentDirection = path.getDirection(path.getSize()-1);
	Vec3 normal(0.0f, 0.0f, 1.0f);
	Vec3 up(0.0f, 1.0f, 0.0f);
	if (parentDirection != up)
		normal = normalize(cross(parentDirection, up));
	normal = normalize(cross(normal, parentDirection));
	float angle = dis(this->mt);

	if (i == 0)
		return rotateAroundAxis(parentDirection, normal, angle);
	else
		return rotateAroundAxis(parentDirection, normal, -angle);
}

Vec3 PatternGenerator::getStemDirection(Stem *stem, const StemData &data, int i)
{
	float variation = data.angleVariation * pi;
	std::uniform_real_distribution<float> dis(-variation, variation);
	float distance = stem->getDistance();
	const Path &path = stem->getParent()->getPath();
	Vec3 parentDirection = path.getIntermediateDirection(distance);
	float ratio = distance / path.getLength();
	float angle = data.leaf.rotation*i + dis(this->mt);
	Quat rotation = fromAxisAngle(parentDirection, angle);

	Vec3 direction;
	if (parentDirection == Vec3(0.0f, 1.0f, 0.0f))
		direction = Vec3(0.0f, 0.0f, 1.0f);
	else
		direction = cross(Vec3(0.0f, 1.0f, 0.0f), parentDirection);
	direction = normalize(direction);
	direction = rotate(rotation, direction);

	dis = std::uniform_real_distribution<float>(ratio*0.5f, ratio);
	return normalize(lerp(direction, parentDirection, dis(this->mt)-0.1f));
}

float getCollarLength(Stem *stem, Vec3 direction, Plant *plant)
{
	float scale = stem->getMaxRadius() * 2.0f;
	if (stem->getParent()) {
		Stem *parent = stem->getParent();
		float distance = stem->getDistance();
		const Path &path = parent->getPath();
		Vec3 parentDirection = path.getIntermediateDirection(distance);
		float difference = std::abs(dot(direction, parentDirection));
		float radius = plant->getIntermediateRadius(parent, distance);
		scale += radius / std::sqrt(1.0f - difference);
	}
	return scale;
}

float PatternGenerator::setPath(Stem *stem, Vec3 direction, float ratio,
	const StemData &data)
{
	float radius = stem->getMaxRadius();
	float length = data.length * radius;
	int points = static_cast<int>(length) + 1;
	float increment = length / points;
	ratio = 1.0f;

	Path path;
	std::vector<Vec3> controls;
	std::uniform_real_distribution<float> dis(-1.0f, 1.0f);

	Vec3 control(0.0f, 0.0f, 0.0f);
	controls.push_back(control);
	control += getCollarLength(stem, direction, this->plant) * direction;
	controls.push_back(control);

	for (int i = 0; i < points; i++) {
		control = control + increment * direction;
		control.x += dis(this->mt) * data.noise;
		control.y += dis(this->mt) * data.noise;
		control.z += dis(this->mt) * data.noise;
		length += magnitude(controls.back() - control);
		controls.push_back(control);

		const float p = dis(this->mt) - (1.0f-data.fork*2.0f);
		if (i < points-1 && p > 0.0f) {
			float radius = stem->getMaxRadius();
			unsigned c = stem->getRadiusCurve();
			Spline spline = this->plant->getCurve(c).getSpline();
			float t = static_cast<float>(i+1);
			t /= static_cast<float>(points);
			radius *= spline.getPoint(t).z;
			if (radius > data.radiusThreshold) {
				ratio = t;
				stem->setMinRadius(radius);
				break;
			}
		}

		float divergence = 1.0f;
		if (dis(this->mt) > 0.0f) {
			float influence = radius;
			if (influence < 0.1f)
				influence = 0.1f;
			divergence = dis(this->mt) * 0.05f;
			divergence /= influence * influence;
		}
		Vec3 change;
		change.x = dis(this->mt) * 0.05f * divergence;
		change.y = dis(this->mt) * 0.05f * divergence;
		change.z = dis(this->mt) * 0.05f * divergence;
		direction = normalize(direction + change);
	}

	Spline spline;
	spline.setDegree(1);
	spline.setControls(controls);
	path.setSpline(spline);
	stem->setPath(path);
	return 1.0f - ratio;
}

void PatternGenerator::addLeaves(Stem *stem, LeafData data, float length)
{
	if (data.density <= 0.0f || data.leavesPerNode < 1)
		return;

	const Path &path = stem->getPath();
	const float distance = 1.0f / data.density;
	float position = path.getLength();
	float end = (length - data.distance);
	end *= (end >= 0.0f);

	for (int i = 0, j = 1; position > end; i++, j++) {
		float t = data.densityCurve.getPoint(position/length).z;
		if (t == 0.0f)
			break;

		Leaf leaf;
		leaf.setPosition(position);
		leaf.setScale(data.scale);
		leaf.setRotation(data, position, path, i);
		stem->addLeaf(leaf);

		if (j >= data.leavesPerNode) {
			position -= distance / t;
			j = 0;
		}
	}
}

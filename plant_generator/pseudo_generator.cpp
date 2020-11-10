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
#include <cstdlib>
#include <cmath>

using namespace pg;

const float pi = 3.14159265359f;

PseudoGenerator::PseudoGenerator(Plant *plant)
{
	this->plant = plant;
}

ParameterTree PseudoGenerator::getParameterTree() const
{
	return this->parameterTree;
}

void PseudoGenerator::setParameterTree(ParameterTree parameterTree)
{
	this->parameterTree = parameterTree;
}

void PseudoGenerator::reset()
{
	ParameterRoot *root = this->parameterTree.getRoot();
	if (root)
		this->randomGenerator.seed(root->getSeed());
}

void PseudoGenerator::grow()
{
	Stem *stem = this->plant->createRoot();
	stem->setParameterTree(this->parameterTree);
	stem->setDistance(0.0f);
	stem->setMaxRadius(0.2f);
	stem->setMinRadius(0.01f);
	stem->setSwelling(Vec2(1.3f, 1.3f));

	const ParameterRoot *root = this->parameterTree.getRoot();
	if (!root)
		return;
	const ParameterNode *node = root->getNode();
	if (!node)
		return;

	setPath(stem, Vec3(0.0f, 1.0f, 0.0f), node->getData());
	reset();
	addStems(stem, node);
}

void PseudoGenerator::grow(Stem *stem)
{
	this->parameterTree = stem->getParameterTree();
	const ParameterRoot *root = this->parameterTree.getRoot();
	if (!root)
		return;
	const ParameterNode *node = root->getNode();
	if (!node)
		return;

	reset();
	addStems(stem, node);
}

void PseudoGenerator::addStems(Stem *stem, const ParameterNode *node)
{
	while (node) {
		addLateralStems(stem, node);
		addLeaves(stem, node->getData().leaf);
		node = node->getSibling();
	}
}

void PseudoGenerator::addLateralStems(Stem *parent, const ParameterNode *node)
{
	StemData stemData = node->getData();
	if (stemData.density == 0.0f)
		return;

	float length = parent->getPath().getLength();
	float distance = 1.0f / stemData.density;
	float position = stemData.start;

	for (int i = 0; position < length; i++) {
		float t = position / length;
		float percentage = stemData.densityCurve.getPoint(t).z;
		if (percentage == 0.0f)
			break;
		addLateralStem(parent, position, node, i);
		position += distance * (1.0f/percentage);
	}
}

void PseudoGenerator::addLateralStem(
	Stem *parent, float position, const ParameterNode *node, int index)
{
	StemData data = node->getData();
	Vec2 swelling(1.2f, 3.0f);
	float radius = this->plant->getIntermediateRadius(parent, position);
	radius = radius / swelling.x * data.scale;
	if (radius < node->getData().radiusThreshold)
		return;

	Stem *stem = plant->addStem(parent);
	stem->setMaxRadius(radius);
	stem->setMinRadius(getMinRadius(radius));
	stem->setSwelling(swelling);
	stem->setDistance(position);
	stem->setSectionDivisions(5);
	Vec3 direction = getStemDirection(stem, data, index);
	setPath(stem, direction, data);

	if (node->getChild())
		addStems(stem, node->getChild());
}

Vec3 PseudoGenerator::getStemDirection(Stem *stem, StemData data, int index)
{
	float variation = data.angleVariation*pi;
	std::uniform_real_distribution<float> dis(-variation, variation);
	float distance = stem->getDistance();
	Path path = stem->getParent()->getPath();
	Vec3 parentDirection = path.getIntermediateDirection(distance);
	float ratio = distance / path.getLength();
	float angle = data.leaf.rotation*index + dis(this->randomGenerator);
	Quat rotation = fromAxisAngle(parentDirection, angle);

	Vec3 direction;
	if (parentDirection == Vec3(0.0f, 1.0f, 0.0f))
		direction = Vec3(0.0f, 0.0f, 1.0f);
	else
		direction = cross(Vec3(0.0f, 1.0f, 0.0f), parentDirection);
	direction = normalize(direction);
	direction = rotate(rotation, direction);

	dis = std::uniform_real_distribution<float>(ratio*0.5f, ratio);
	float t = dis(this->randomGenerator) - 0.1f;
	return normalize(lerp(direction, parentDirection, t));
}

float getCollarLength(Stem *stem, Vec3 direction, Plant *plant)
{
	float scale = stem->getMaxRadius() * 2.0f;
	if (stem->getParent()) {
		Stem *parent = stem->getParent();
		float distance = stem->getDistance();
		Path path = parent->getPath();
		Vec3 parentDirection = path.getIntermediateDirection(distance);
		float difference = std::abs(dot(direction, parentDirection));
		float radius = plant->getIntermediateRadius(parent, distance);
		scale += radius / std::sqrt(1.0f - difference);
	}
	return scale;
}

void PseudoGenerator::setPath(Stem *stem, Vec3 direction, StemData data)
{
	float radius = stem->getMaxRadius();
	float length = radius * data.length;
	int points = static_cast<int>(length) + 1;
	float increment = length / points;

	Path path;
	std::vector<Vec3> controls;
	std::uniform_real_distribution<float> dis(-0.05f, 0.05f);

	Vec3 control(0.0f, 0.0f, 0.0f);
	controls.push_back(control);
	control += getCollarLength(stem, direction, this->plant) * direction;
	controls.push_back(control);

	for (int i = 0; i < points; i++) {
		control = control + increment * direction;
		control.x += dis(this->randomGenerator);
		control.y += dis(this->randomGenerator);
		control.z += dis(this->randomGenerator);
		controls.push_back(control);

		float divergence = 1.0f;
		if (dis(this->randomGenerator) > 0.0f) {
			float influence = radius;
			if (influence < 0.1f)
				influence = 0.1f;
			divergence = dis(this->randomGenerator);
			divergence /= influence * influence;
		}
		Vec3 change;
		change.x = dis(this->randomGenerator) * divergence;
		change.y = dis(this->randomGenerator) * divergence;
		change.z = dis(this->randomGenerator) * divergence;
		direction = normalize(direction + change);
	}

	Spline spline;
	spline.setDegree(1);
	spline.setControls(controls);
	path.setSpline(spline);
	stem->setPath(path);
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

void PseudoGenerator::addLeaves(Stem *stem, LeafData data)
{
	if (data.density <= 0.0f || data.leavesPerNode < 1)
		return;

	Path path = stem->getPath();
	float length = path.getLength();
	float start = length - data.distance;
	float distance = 1.0f / data.density;
	float position = length;

	if (start < 0.0f)
		start = 0.0f;

	Vec3 zaxis(0.0f, 0.0f, 1.0f);
	Vec3 yaxis(0.0f, 1.0f, 0.0f);
	Vec3 xaxis(1.0f, 0.0f, 0.0f);

	for (int i = 0, j = 1; position > start; i++, j++) {
		Quat rotation(0.0f, 0.0f, 0.0f, 1.0f);
		Vec3 stemDirection = path.getIntermediateDirection(position);
		float ratio = (length - position) / (length - start);
		Vec3 normal;
		Vec3 normal1;
		Vec3 normal2;
		float mix;

		/* Rotate the leaf into the stem side. */
		if (stemDirection == yaxis)
			normal = zaxis;
		else
			normal = normalize(cross(yaxis, stemDirection));
		Quat angle = fromAxisAngle(stemDirection, data.rotation*i);
		rotation = rotateIntoVecQ(zaxis, normal);
		rotation = angle * rotation;

		/* Rotate the leaf normal into the stem direction. */
		normal = rotate(rotation, yaxis);
		rotation = rotateIntoVecQ(normal, stemDirection) * rotation;

		/* Rotate the leaf normal into the up vector. */
		mix = data.minUp + (data.maxUp - data.minUp) * ratio;
		normal1 = rotate(rotation, yaxis);
		normal2 = normalize(lerp(yaxis, stemDirection, mix));
		rotation = rotateIntoVecQ(normal1, normal2) * rotation;

		/* Rotate the leaf direction into the stem. */
		mix = data.minDirection;
		mix += (data.maxDirection - data.minDirection) * ratio;
		normal1 = rotate(rotation, zaxis);
		normal2 = normalize(lerp(stemDirection, normal1, mix));
		rotation = rotateIntoVecQ(normal1, normal2) * rotation;

		float t = position / length;
		float percentage = data.densityCurve.getPoint(t).z;
		if (percentage == 0.0f)
			break;

		addLeaf(stem, data, position, normalize(rotation));

		if (j == data.leavesPerNode) {
			position -= distance * (1.0f/percentage);
			j = 0;
		}
	}
}

void PseudoGenerator::addLeaf(
	Stem *stem, LeafData data, float position, Quat rotation)
{
	Leaf leaf;
	leaf.setPosition(position);
	leaf.setScale(data.scale);
	leaf.setRotation(rotation);
	stem->addLeaf(leaf);
}

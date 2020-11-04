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
	addLeaves(stem, root->getData());

	reset();
	while (node) {
		addLateralStems(stem, node);
		node = node->getSibling();
	}
}

void PseudoGenerator::grow(Stem *stem)
{
	this->parameterTree = stem->getParameterTree();

	const ParameterRoot *root = this->parameterTree.getRoot();
	if (!root)
		return;

	addLeaves(stem, root->getData());

	const ParameterNode *node = root->getNode();
	if (!node)
		return;

	reset();
	while (node) {
		addLateralStems(stem, node);
		node = node->getSibling();
	}
}

Vec3 PseudoGenerator::getStemDirection(Stem *stem)
{
	Path path = stem->getParent()->getPath();
	float ratio = stem->getDistance() / path.getLength();
	Vec3 direction = path.getIntermediateDirection(stem->getDistance());
	std::uniform_real_distribution<float> dis(0.0f, 2.0*pi);
	float angleX = pi*(0.1f + 0.4f*(1.0f-ratio));
	float angleY = dis(this->randomGenerator);
	Mat4 rot = pg::rotateXY(angleX, angleY);
	Mat4 tran = pg::rotateIntoVec(Vec3(0.0f, 1.0f, 0.0f), direction);
	tran = tran * rot;
	return tran.apply(Vec3(0.0f, 1.0f, 0.0f), 1.0f);
}

void PseudoGenerator::addLateralStems(Stem *parent, const ParameterNode *node)
{
	StemData stemData = node->getData();
	if (stemData.density == 0.0f)
		return;

	float length = parent->getPath().getLength();
	float distance = 1.0f / stemData.density;
	float position = stemData.start;

	while (position < length) {
		float t = position / length;
		float percentage = stemData.densityCurve.getPoint(t).z;
		if (percentage == 0.0f)
			break;
		addLateralStem(parent, position, node);
		position += distance * (1.0f/percentage);
	}
}

void PseudoGenerator::addLateralStem(
	Stem *parent, float position, const ParameterNode *node)
{
	Vec2 swelling(1.2f, 3.0f);
	float radius = getRadius(parent, 1.5f, position);
	if (radius < node->getData().radiusThreshold)
		return;

	Stem *stem = plant->addStem(parent);
	stem->setMaxRadius(radius);
	stem->setMinRadius(getMinRadius(radius));
	stem->setSwelling(swelling);
	stem->setDistance(position);
	stem->setSectionDivisions(5);
	setPath(stem, getStemDirection(stem), node->getData());
	addLeaves(stem, node->getData().leaf);

	if (node->getChild())
		addLateralStems(stem, node->getChild());
}

float PseudoGenerator::getRadius(Stem *parent, float margin, float position)
{
	float r = this->plant->getIntermediateRadius(parent, position);
	return r / margin;
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

void PseudoGenerator::setPath(Stem *stem, Vec3 direction, StemData stemData)
{
	float radius = stem->getMaxRadius();
	float length = radius * stemData.lengthFactor;
	int points = static_cast<int>(length / 2.0f) + 1;
	float increment = length / points;

	Path path;
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

void PseudoGenerator::addLeaves(Stem *stem, LeafData data)
{
	if (data.density <= 0.0f || data.leavesPerNode < 1)
		return;

	Path path = stem->getPath();
	float length = path.getLength();
	float start = length - data.distance;
	float distance = 1.0f / data.density;
	float position = length;
	Quat rotation(0.0f, 0.0f, 0.0f, 1.0f);

	if (start < 0.0f)
		start = 0.0f;

	Vec3 zaxis(0.0f, 0.0f, 1.0f);
	Vec3 yaxis(0.0f, 1.0f, 0.0f);
	Vec3 xaxis(1.0f, 0.0f, 0.0f);

	for (int i = 0, j = 1; position > start; i++, j++) {
		rotation = Quat(0.0f, 0.0f, 0.0f, 1.0f);
		Vec3 stemDirection = path.getIntermediateDirection(position);
		float ratio = (length - position) / (length - start);
		Vec3 normal;
		Vec3 normal1;
		Vec3 normal2;
		float mix;

		/* Rotate the leaf into the stem side. */
		normal = pg::normalize(pg::cross(yaxis, stemDirection));
		rotation = pg::rotateIntoVecQ(zaxis, normal);
		Quat angle = fromAxisAngle(stemDirection, data.rotation*i);
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

		addLeaf(stem, data, position, pg::normalize(rotation));

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

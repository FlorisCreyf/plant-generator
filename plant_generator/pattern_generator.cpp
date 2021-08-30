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
		this->maxDepth = root->getData().maxDepth;
		this->mt.seed(root->getData().seed);
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
		Vec3 d(0.0f, 0.0f, 1.0f);
		float l = getCollarLength(stem, d);
		float ratio = setPath(stem, 0.0f, d, l, root->getData());
		addStems(stem, ratio, 0.0f, root);
	}
}

void PatternGenerator::grow(Stem *stem)
{
	this->parameterTree = stem->getParameterTree();
	ParameterNode *root = this->parameterTree.getRoot();
	if (root) {
		reset();
		const Path &path = stem->getPath();
		Vec3 d = path.getDirection(0);
		float l = getCollarLength(stem, d);
		float ratio = setPath(stem, 0.0f, d, l, root->getData());
		addStems(stem, ratio, 0.0f, root);
	}
}

float getForkCollarLength(Vec3 direction1, Vec3 direction2, float radius)
{
	float t = 0.5f * std::acos(dot(direction1, direction2));
	return std::sin(0.5f*pi-t) / std::sin(t) * radius * 1.1f;
}

float PatternGenerator::addStems(Stem *stem, float ratio, float length,
	const ParameterNode *node)
{
	const StemData &data = node->getData();
	float totalLength = length + stem->getPath().getLength();

	if (!stem->isCustom() && ratio <= 0.0f)
		stem->setMinRadius(0.0f);
	else {
		float radius = stem->getMinRadius() * data.forkScale;
		Vec3 d1 = getForkDirection(stem, 1.0f, data);
		Vec3 d2 = getForkDirection(stem, -1.0f, data);
		float collarLength = getForkCollarLength(d1, d2, radius);

		Stem *fork1 = plant->addStem(stem);
		fork1->setMaxRadius(radius);
		fork1->setDistance(std::numeric_limits<float>::max());
		fork1->setSectionDivisions(stem->getSectionDivisions());
		ratio = setPath(fork1, ratio, d1, collarLength, data);
		float l1 = addStems(fork1, ratio, totalLength, node);

		Stem *fork2 = plant->addStem(stem);
		fork2->setMaxRadius(radius);
		fork2->setDistance(std::numeric_limits<float>::max());
		fork2->setSectionDivisions(stem->getSectionDivisions());
		ratio = setPath(fork2, ratio, d2, collarLength, data);
		float l2 = addStems(fork2, ratio, totalLength, node);

		totalLength = l1 > l2 ? l1 : l2;
	}

	node = node->getChild();
	while (node) {
		Length l(length, totalLength);
		addLateralStems(stem, l, node);
		addLeaves(stem, l, node->getData().leaf);
		node = node->getSibling();
	}

	return totalLength;
}

void PatternGenerator::addLateralStems(Stem *parent, Length length,
	const ParameterNode *node)
{
	StemData stemData = node->getData();
	if (stemData.density == 0.0f)
		return;

	const Path &path = parent->getPath();
	float distance = 1.0f / stemData.density;
	float position = path.getLength() - distance * 0.5f;
	float end = length.total - length.current - stemData.distance;
	if (end <= distance)
		end = distance * 0.5f;

	Vec3 d1(0.0f, 1.0f, 0.0f);
	Vec3 d2 = path.getIntermediateDirection(end);
	if (d2 != Vec3(0.0f, 0.0f, 1.0f))
		d1 = cross(Vec3(0.0f, 0.0f, 1.0f), d2);

	for (int i = 0; position > end; i++) {
		float t = position / length.total;
		float r = stemData.densityCurve.getPoint(t).y;
		if (r == 0.0f)
			break;
		addLateralStem(parent, position, length, i, d1, d2, node);
		position -= distance * (1.0f/r);
	}
}

void PatternGenerator::addLateralStem(Stem *parent, float position,
	Length length, int index, Vec3 &direction1, Vec3 &direction2,
	const ParameterNode *node)
{
	StemData data = node->getData();
	Vec2 collar(1.5f, 3.0f);
	float radius = this->plant->getIntermediateRadius(parent, position);
	radius = modifyRadius(data, radius / collar.x);
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

	const Path &path = parent->getPath();
	float distance = stem->getDistance();
	Vec3 d = path.getIntermediateDirection(distance);
	Quat r = rotateIntoVecQ(direction2, d);
	direction2 = d;
	direction1 = rotate(r, direction1);

	d = getDirection(stem, index, length, direction1, direction2, data);
	float collarLength = getCollarLength(stem, d);
	float ratio = (stem->getDistance() + length.current) / length.total;
	ratio = setPath(stem, ratio, d, collarLength, data);
	addStems(stem, ratio, collarLength, node);
}

float PatternGenerator::modifyRadius(const StemData &data, float radius)
{
	std::normal_distribution<float> dis(1.0f, data.radiusVariation);
	float variation = dis(this->mt);
	if (variation > 1.0f)
		variation = 1.0f;
	return radius * data.radius * variation;
}

Vec3 PatternGenerator::getDirection(Stem *stem, int index, Length length,
	Vec3 direction1, Vec3 direction2, const StemData &data)
{
	float variation = data.angleVariation * pi;
	std::uniform_real_distribution<float> dis1(-variation, variation);
	float ratio = (stem->getDistance() + length.current) / length.total;
	float radialAngle = data.leaf.rotation*index + dis1(this->mt);
	Quat radialRotation = fromAxisAngle(direction2, radialAngle);
	direction1 = normalize(direction1);
	direction1 = rotate(radialRotation, direction1);

	ratio = data.inclineCurve.getPoint(ratio).y;
	float t = 2.0f * (ratio - 0.5f);
	std::normal_distribution<float> dis2(0.0f, data.inclineVariation);
	t += dis2(this->mt);
	if (t < 0.0f) {
		t *= -1.0f;
		direction2 *= -1.0f;
	}
	if (t > 0.9f)
		t = 0.9f;

	return normalize(lerp(direction1, direction2, t));
}

Vec3 PatternGenerator::getForkDirection(Stem *stem, float sign,
	const StemData &data)
{
	float minAngle = 0.1f;
	float maxAngle = data.forkAngle;
	if (data.forkAngle < minAngle)
		maxAngle = minAngle;

	std::uniform_real_distribution<float> dis(minAngle, maxAngle);
	const Path &path = stem->getPath();
	Vec3 parentDirection = path.getDirection(path.getSize()-1);
	Vec3 normal(0.0f, 1.0f, 0.0f);
	Vec3 up(0.0f, 0.0f, 1.0f);
	if (parentDirection != up)
		normal = normalize(cross(parentDirection, up));
	normal = normalize(cross(normal, parentDirection));
	float angle = sign * dis(this->mt);

	return rotateAroundAxis(parentDirection, normal, angle);
}

float PatternGenerator::getCollarLength(Stem *stem, Vec3 direction)
{
	float scale = stem->getMaxRadius() * 2.0f;
	if (stem->getParent()) {
		Stem *parent = stem->getParent();
		float distance = stem->getDistance();
		const Path &path = parent->getPath();
		Vec3 parentDirection = path.getIntermediateDirection(distance);
		float d = std::abs(dot(direction, parentDirection));
		float r = this->plant->getIntermediateRadius(parent, distance);
		scale += r / std::sqrt(1.0f - d);
	}
	return scale;
}

float PatternGenerator::setPath(Stem *stem, float ratio, Vec3 direction,
	float collarLength, const StemData &data)
{
	if (stem->isCustom())
		return 1.0f;

	std::vector<Vec3> controls;
	Vec3 control(0.0f);
	controls.push_back(control);
	control += collarLength * direction;
	controls.push_back(control);

	Path path;
	std::normal_distribution<float> dis(0.0f, 1.0f);
	float radius = stem->getMaxRadius();
	float length = data.length * data.lengthCurve.getPoint(ratio).y;
	length *= radius;
	int points = length * data.pointDensity;
	if (points < 2)
		points = 2;
	float increment = length / points;
	ratio = 1.0f;

	for (int i = 0; i < points; i++) {
		if (i < points-1) {
			float t = i*increment / length;
			ratio = bifurcatePath(stem, t, data);
		}
		if (ratio != 1.0f) {
			increment = 2.0f * radius;
			control = control + increment * direction;
			controls.push_back(control);
			break;
		}

		Vec3 change = direction;
		change.x += dis(this->mt) * data.noise;
		change.y += dis(this->mt) * data.noise;
		change.z += dis(this->mt) * data.noise;
		control += increment * normalize(change);
		controls.push_back(control);

		float scale = 0.1f/(1.0f+pi*radius*radius*length);
		float pull = sqrt(control.x*control.x + control.y*control.y);
		change.x = dis(this->mt) * scale;
		change.y = dis(this->mt) * scale;
		change.z = dis(this->mt) * scale;
		change.z -= data.gravity * pull;
		direction = normalize(direction + change);
	}

	Spline spline;
	spline.setDegree(1);
	spline.setControls(controls);
	path.setSpline(spline);
	stem->setPath(path);
	return 1.0f - ratio;
}

float PatternGenerator::bifurcatePath(Stem *stem, float ratio,
	const StemData &data)
{
	int depth = stem->getDepth();
	std::bernoulli_distribution dis(data.fork);
	if (dis(this->mt) && depth <= this->maxDepth) {
		float radius = stem->getMaxRadius();
		unsigned curve = stem->getRadiusCurve();
		Spline spline = this->plant->getCurve(curve).getSpline();
		radius *= spline.getPoint(ratio).y;
		if (radius > data.radiusThreshold) {
			stem->setMinRadius(radius);
			return ratio;
		}
	}
	return 1.0f;
}

void PatternGenerator::addLeaves(Stem *stem, Length length, LeafData data)
{
	if (data.density <= 0.0f || data.leavesPerNode < 1)
		return;

	const Path &path = stem->getPath();
	const float distance = 1.0f / data.density;
	float position = path.getLength();
	float end = length.total - length.current - data.distance;
	end *= (end >= 0.0f);

	for (int i = 0, j = 1; position > end; i++, j++) {
		float ratio = position / length.total;
		float t = data.densityCurve.getPoint(ratio).y;
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

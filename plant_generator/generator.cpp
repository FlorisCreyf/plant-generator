/* Copyright 2020 Floris Creyf
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

#include "generator.h"
#include <cmath>
#include <limits>

using namespace pg;
using std::cos;
using std::sin;
using std::vector;

const float pi = 3.14159265359f;

Generator::Generator(Plant *plant) :
	plant(plant),
	primaryGrowthRate(0.1f),
	secondaryGrowthRate(0.001f),
	minRadius(0.001f),
	rayCount(20),
	rayLevels(10),
	width(0.0f)
{
	Stem *root = this->plant->createRoot();

	Path path;
	Spline spline;
	std::vector<Vec3> controls;
	controls.push_back(Vec3(0.0f, 0.0f, 0.0f));
	spline.setControls(controls);
	spline.setDegree(1);
	path.setSpline(spline);
	root->setPath(path);
	root->setSectionDivisions(6);
	root->setMinRadius(this->minRadius);
	root->setMaxRadius(this->minRadius);
	root->setSwelling(Vec2(1.0f, 1.0f));

	updateBoundingBox(Vec3(0.0f, 0.0f, 0.0f));
}

Generator::Generator(const Generator &original) :
	plant(original.plant),
	growth(original.growth),
	primaryGrowthRate(original.primaryGrowthRate),
	secondaryGrowthRate(original.secondaryGrowthRate),
	minRadius(original.minRadius),
	rayCount(original.rayCount),
	rayLevels(original.rayLevels),
	width(original.width)
{

}

Generator &Generator::operator=(const Generator &original)
{
	this->plant = original.plant;
	this->growth = original.growth;
	this->primaryGrowthRate = original.primaryGrowthRate;
	this->secondaryGrowthRate = original.secondaryGrowthRate;
	this->minRadius = original.minRadius;
	this->rayCount = original.rayCount;
	this->rayLevels = original.rayLevels;
	this->width = original.width;
	return *this;
}

void Generator::grow(int cycles, int nodes)
{
	for (int i = 0; i < cycles; i++) {
		if (i > 0)
			addStems(this->plant->getRoot());
		for (int j = 0; j < nodes; j++)
			addNodes(j);
	}
}

void Generator::addNodes(int node)
{
	Stem *root = this->plant->getRoot();
	this->growth.clear();
	castRays();
	propagate(root);
	for (auto &pair : this->growth)
		addNode(pair.first, pair.second, node);
}

/** Rays are used to estimate how much light each leaf receives. */
void Generator::castRays()
{
	for (int i = 1; i <= this->rayLevels; i++) {
		float angle = i / (float)this->rayLevels * pi * 0.5f;
		int rayCount = (this->rayCount-1) * std::cos(angle) + 1;
		float y = std::sin(angle) * this->width;
		float radius = this->width * std::cos(angle);

		for (int j = 0; j < rayCount; j++) {
			float angle = j*(2.0f*pi/rayCount);
			float x = std::cos(angle) * radius;
			float z = std::sin(angle) * radius;
			Vec3 origin(-x, 0.0f, -z);
			Ray ray;
			ray.origin = Vec3(x, y, z);
			ray.direction = normalize(origin-ray.origin);
			updateGrowth(ray);
		}
	}
}

void Generator::updateGrowth(Ray ray)
{
	Intersection intersection = intersect(this->plant->getRoot(), ray);
	Stem *stem = intersection.stem;
	if (stem) {
		if (this->growth.find(stem) == this->growth.end()) {
			Light light = {};
			this->growth[stem] = light;
		}
		this->growth[stem].direction += ray.direction;
		this->growth[stem].rays++;
	}
}

void Generator::addNode(Stem *stem, Light light, int node)
{
	light.direction /= light.rays;
	light.direction = normalize(light.direction);

	Path path = stem->getPath();
	Spline spline = path.getSpline();
	std::vector<Vec3> controls = spline.getControls();

	Vec3 point = controls.back();
	Vec3 direction = -1.0f * light.direction;
	size_t pathSize = path.getSize();

	if (pathSize > 1) {
		Vec3 lastDirection = path.getDirection(path.getSize()-1);
		float max = std::sqrt(2.0f) / 2.0f;
		direction = clamp(direction, lastDirection, max);
	}

	/* The first node already has a leaf in order to determine the initial
	direction of the stem. */
	bool firstNode = stem->getParent() && pathSize == 2 && node == 0;
	if (firstNode) {
		Vec3 initDirection = path.getDirection(path.getSize()-1);
		Vec3 normal = cross(initDirection, direction);
		direction = projectOntoPlane(direction, normal);
		direction = normalize(direction);
		controls[1] = this->primaryGrowthRate * direction;
	} else {
		point += this->primaryGrowthRate * direction;
		controls.push_back(point);
	}
	updateBoundingBox(point + stem->getLocation());

	spline.setControls(controls);
	path.setSpline(spline);
	stem->setPath(path);
	stem->setMaxRadius(stem->getMaxRadius()+this->secondaryGrowthRate);
	stem->setSwelling(Vec2(1.5f, 1.5f));

	if (!firstNode)
		addLeaves(stem, stem->getPath().getLength());
}

Generator::Intersection Generator::intersect(Stem *stem, Ray ray)
{
	Intersection intersection;
	intersection.stem = nullptr;
	intersection.t = std::numeric_limits<float>::max();
	Path path = stem->getPath();

	Stem *child = stem->getChild();
	while (child != nullptr) {
		Intersection i = intersect(child, ray);
		if (i.stem && i.t < intersection.t)
			intersection = i;
		child = child->getSibling();
	}
	for (const Leaf &leaf : stem->getLeaves()) {
		float radius = leaf.getScale().x * 0.5f;
		float distance = leaf.getPosition();
		Vec3 location = stem->getLocation();
		Vec3 direction(0.0f, 0.0f, 1.0f);
		if (path.getSize() > 1) {
			location += path.getIntermediate(distance);
			direction = path.getIntermediateDirection(distance);
		}
		direction = rotate(leaf.getRotation(), direction);
		location += radius * direction;

		float t = intersectsSphere(ray, location, radius);
		if (t != 0.0f && (!intersection.stem || t < intersection.t)) {
			intersection.stem = stem;
			intersection.t = t;
		}
	}

	return intersection;
}

/** Growth of parent stems is dependent on the efficiency of child stems. */
int Generator::propagate(Stem *stem)
{
	Light light = {};
	int accumulatedLight = 0;

	if (this->growth.find(stem) != this->growth.end()) {
		light = this->growth.at(stem);
		accumulatedLight = light.rays;
	}

	Stem *child = stem->getChild();
	while (child) {
		Stem *sibling = child->getSibling();
		accumulatedLight += propagate(child);
		child = sibling;
	}

	if (light.rays > 0 && magnitude(light.direction) > 0.1f) {
		this->growth[stem] = light;
	} else if (stem->getParent()) {
		Path path = stem->getPath();
		if (path.getSize() > 1 && accumulatedLight > 0) {
			Vec3 direction = path.getDirection(path.getSize()-1);
			light.direction = direction;
			light.rays = 1;
			this->growth[stem] = light;
		} else {
			this->growth.erase(stem);
			this->plant->deleteStem(stem);
		}
	} else {
		Path path = stem->getPath();
		if (path.getSize() > 1)
			light.direction = path.getDirection(path.getSize()-1);
		else
			light.direction = Vec3(0.0f, -1.0f, 0.0f);
		light.rays = 1;
		this->growth[stem] = light;
	}

	return accumulatedLight;
}

Vec3 getInitialDirection(Stem *stem, Leaf leaf)
{
	Path path = stem->getPath();
	float distance = leaf.getPosition();
	Vec3 direction = path.getIntermediateDirection(distance);
	if (direction == Vec3(0.0f, 1.0f, 0.0f))
		direction = Vec3(0.0f, 0.0f, 1.0f);
	else {
		direction = cross(Vec3(0.0f, 1.0f, 0.0f), direction);
		direction = normalize(direction);
	}
	return rotate(leaf.getRotation(), direction);
}

void Generator::addStems(Stem *stem)
{
	Stem *child = stem->getChild();
	while (child) {
		addStems(child);
		child = child->getSibling();
	}

	std::vector<Leaf> leaves = stem->getLeaves();
	for (auto it = leaves.begin(); it != leaves.end(); it++) {
		Leaf leaf = *it;

		/* TODO: Not all leaves should be removed. */
		stem->removeLeaf(0);

		Vec3 direction = getInitialDirection(stem, leaf);
		Vec3 point = this->primaryGrowthRate * direction;

		Stem *child = this->plant->addStem(stem);
		child->setDistance(leaf.getPosition());
		child->setSwelling(Vec2(1.0f, 1.0f));
		Path path;
		Spline spline = path.getSpline();
		spline.addControl(Vec3(0.0f, 0.0f, 0.0f));
		spline.addControl(point);
		spline.setDegree(1);
		path.setSpline(spline);
		child->setPath(path);
		child->setMinRadius(this->minRadius);
		child->setMaxRadius(this->minRadius);

		Leaf childLeaf = createLeaf();
		childLeaf.setPosition(this->primaryGrowthRate);
		child->addLeaf(childLeaf);
	}
}

void Generator::addLeaves(Stem *stem, float distance)
{
	Quat rotation(0.0f, 0.0f, 0.0f, 0.0f);
	auto leaves = stem->getLeaves();
	if (leaves.empty())
		rotation.w = 1.0f;
	else {
		Leaf prevLeaf = *leaves.rbegin();
		if (prevLeaf.getRotation().w == 1.0f)
			rotation.y = 1.0f;
		else
			rotation.w = 1.0f;
	}
	Leaf leaf = createLeaf();
	leaf.setPosition(distance);
	leaf.setRotation(rotation);
	stem->addLeaf(leaf);
}

Leaf Generator::createLeaf()
{
	Leaf leaf;
	leaf.setScale(Vec3(0.04f, 0.04f, 0.04f));
	leaf.setMesh(0);
	return leaf;
}

/** A bounding box is created to determine how rays should be generated. */
void Generator::updateBoundingBox(Vec3 point)
{
	point.x = point.x * 2.0f + std::copysign(0.1f, point.x);
	point.y = point.y * 2.0f + std::copysign(0.1f, point.y);
	point.z = point.z * 2.0f + std::copysign(0.1f, point.z);
	if (point.x > this->width)
		this->width = point.x;
	if (point.y > this->width)
		this->width = point.y;
	if (point.z > this->width)
		this->width = point.z;
}

void Generator::setPrimaryGrowthRate(float rate)
{
	if (rate > 0.0f)
		this->primaryGrowthRate = rate;
}

void Generator::setSecondaryGrowthRate(float rate)
{
	if (rate > 0.0f)
		this->secondaryGrowthRate = rate;
}

void Generator::setRayDensity(int baseCount, int levels)
{
	if (baseCount > 0)
		this->rayCount = baseCount;
	if (levels > 0)
		this->rayLevels = levels;
}

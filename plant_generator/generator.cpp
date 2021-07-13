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
	width(0.0f),
	primaryGrowthRate(0.5f),
	secondaryGrowthRate(0.005f),
	minRadius(0.001f),
	suppression(0.0025f),
	synthesisRate(0.001f),
	synthesisThreshold(0.5f),
	depth(2),
	rays(10000),
	cycles(5),
	nodes(4),
	seed(0)
{

}

void Generator::grow()
{
	this->mt.seed(this->seed);
	this->width = 1.0f;
	Stem *root = createRoot();
	for (int i = 0; i < this->cycles; i++) {
		if (i > 0) {
			evaluateEfficiency(&this->volume, root);
			addStems(root, &this->volume);
		}

		for (int j = 0; j < this->nodes; j++) {
			int depth = std::log2(this->width) + this->depth;
			if (depth <= 0)
				depth = 1;
			this->volume.clear(this->width*2.0f, depth);
			addToVolume(&this->volume, root);
			generalizeDensity(this->volume.getRoot());
			setConcentration(root);
			castRays(&this->volume);
			generalizeFlux(this->volume.getRoot());
			addNodes(&this->volume, root, j, nodes);
		}
	}
}

Stem *Generator::createRoot()
{
	Path path;
	Spline spline;
	std::vector<Vec3> controls;
	controls.push_back(Vec3(0.0f, 0.0f, 0.0f));
	controls.push_back(Vec3(0.0f, 0.0f, this->primaryGrowthRate/2.0f));
	spline.setControls(controls);
	spline.setDegree(1);
	path.setSpline(spline);

	Stem *root = this->plant->createRoot();
	root->setPath(path);
	root->setSectionDivisions(6);
	root->setMinRadius(this->minRadius);
	root->setMaxRadius(this->minRadius);
	root->setSwelling(Vec2(1.5f, 1.5f));

	updateBoundingBox(controls[0]);
	updateBoundingBox(controls[1]);
	return root;
}

void Generator::addToVolume(Volume *volume, Stem *stem)
{
	const Path &path = stem->getPath();
	Vec3 position = stem->getLocation();
	for (size_t i = 1; i < path.getSize(); i++) {
		Vec3 a = position + path.get(i-1);
		Vec3 b = position + path.get(i);
		float radius = this->plant->getRadius(stem, i);
		volume->addLine(a, b, 1.0f, radius);
	}
	Stem *child = stem->getChild();
	while (child) {
		addToVolume(volume, child);
		child = child->getSibling();
	}
}

void Generator::castRays(Volume *volume)
{
	float w = this->width - 0.0001f;
	std::uniform_real_distribution<float> dis1(-w, w);
	std::uniform_real_distribution<float> dis2(-1.0f, 1.0f);
	for (int i = 1; i <= this->rays; i++) {
		float x = dis1(this->mt);
		float y = dis1(this->mt);
		float z = this->width*1.5f;
		Vec3 origin(x, y, z);
		Ray ray;
		ray.origin = Vec3(x, y, z);
		x = dis2(this->mt);
		y = dis2(this->mt);
		z = -1.0f;
		ray.direction = normalize(Vec3(x, y, z));
		updateRadiantEnergy(volume, ray);
	}
}

void Generator::updateRadiantEnergy(Volume *volume, Ray ray)
{
	float magnitude = 1.0f;
	Volume::Node *node = nullptr;
	Volume::Node *nextNode = volume->getNode(ray.origin);
	while (nextNode && node != nextNode) {
		node = nextNode;
		node->setQuantity(node->getQuantity() + 1);
		Vec3 direction = magnitude * ray.direction;
		node->setDirection(node->getDirection() + direction);
		magnitude -= node->getDensity();
		if (magnitude < 0.0f)
			magnitude = 0.0f;
		nextNode = node->getAdjacentNode(ray);
	}
}

float Generator::setConcentration(Stem *stem)
{
	Stem *child = stem->getChild();
	float total = this->suppression*this->suppression*pi;
	while (child) {
		total += setConcentration(child);
		child->getState()->concentration = total;
		child = child->getSibling();
	}
	return total;
}

void Generator::generalizeDensity(Volume::Node *node)
{
	float density = 0.0f;
	for (int i = 0; i < 8; i++) {
		Volume::Node *child = node->getNode(i);
		if (child->getNode(0))
			generalizeDensity(child);
		density += child->getDensity();
	}
	node->setDensity(density / 8.0f);
}

void Generator::generalizeFlux(Volume::Node *node)
{
	Vec3 direction(0.0f, 0.0f, 0.0f);
	float count = 0.0f;
	for (int i = 0; i < 8; i++) {
		Volume::Node *n = node->getNode(i);
		if (n->getNode(0))
			generalizeFlux(n);
		else if (n->getQuantity() > 0) {
			Vec3 f = n->getDirection() / n->getQuantity();
			float m = magnitude(f);
			if (m > 1.0f)
				n->setDirection(f/m);
			else
				n->setDirection(f);
		}
		if (!isZero(n->getDirection())) {
			count += 1.0f;
			direction += n->getDirection();
		}
	}
	if (count > 0.0f)
		node->setDirection(direction / count);
}

float Generator::evaluateEfficiency(Volume *volume, Stem *stem)
{
	float total = 0.0f;

	for (size_t i = 0; i < stem->getLeafCount(); i++) {
		const Leaf *leaf = stem->getLeaf(i);
		Vec3 location = stem->getLocation();
		float position = leaf->getPosition();
		location += stem->getPath().getIntermediate(position);
		Volume::Node *node = volume->getNode(location);
		total += magnitude(node->getDirection()) * this->synthesisRate;
	}

	Stem *child = stem->getChild();
	while (child) {
		Stem *sibling = child->getSibling();
		total += evaluateEfficiency(volume, child);
		child = sibling;
	}

	float r = stem->getMaxRadius();
	float l = stem->getPath().getLength();
	float p = total/(total + l*r*r);
	if (stem->getParent() && p < this->synthesisThreshold)
		this->plant->deleteStem(stem);

	return total;
}

void Generator::addNodes(Volume *volume, Stem *stem, int i, int n)
{
	addNode(volume, stem, i, n);
	Stem *child = stem->getChild();
	while (child) {
		addNodes(volume, child, i, n);
		child = child->getSibling();
	}
}

void Generator::updateRadius(Stem *stem)
{
	stem->setMaxRadius(stem->getMaxRadius() + this->secondaryGrowthRate);
}

Vec3 getDirection(Stem *stem, Vec3 origin, Vec3 direction, Volume *volume)
{
	Volume::Node *node = volume->getNode(stem->getLocation() + origin);
	Vec3 d(0.0f, 0.0f, 0.0f);
	while (node && isZero(d)) {
		d += node->getDirection();
		node = node->getParent();
	}
	return normalize(direction - d);
}

void Generator::addNode(Volume *volume, Stem *stem, int i, int n)
{
	float rate = 1.0f;
	if (stem->getParent()) {
		Stem *parent = stem->getParent();
		float distance = stem->getDistance();
		float r = this->plant->getIntermediateRadius(parent, distance);
		rate = 1.0f - stem->getState()->concentration / (r*r*pi);
		rate *= (rate > 0.0f);
		if (rate < 0.5f || static_cast<int>(rate*n) < i)
			return;
		if (r >= 1.2f * stem->getSwelling().x * stem->getMaxRadius())
			updateRadius(stem);
		else
			return;
	} else
		updateRadius(stem);

	Path path = stem->getPath();
	Spline spline = path.getSpline();
	std::vector<Vec3> controls = spline.getControls();

	Ray ray;
	ray.origin = controls.back();
	ray.direction = path.getDirection(path.getSize()-1);
	Vec3 direction = getDirection(stem, ray.origin, ray.direction, volume);
	Vec3 point = ray.origin + rate * this->primaryGrowthRate * direction;

	controls.push_back(point);
	spline.setControls(controls);
	path.setSpline(spline);
	stem->setPath(path);

	updateBoundingBox(point + stem->getLocation());
	addLeaves(stem, stem->getState()->node++);
}

Vec3 getInitialDirection(Leaf leaf, Stem *stem, Volume *volume)
{
	Vec3 d = rotate(leaf.getRotation(), Vec3(0.0f, 1.0f, 0.0f));
	return getDirection(stem, Vec3(0.0f, 0.0f, 0.0f), d, volume);
}

void Generator::addStems(Stem *stem, Volume *volume)
{
	Stem *child = stem->getChild();
	while (child) {
		addStems(child, volume);
		child = child->getSibling();
	}
	if (stem->getDepth() < 4)
		addStem(stem, volume);
}

void Generator::addStem(Stem *stem, Volume *volume)
{
	for (size_t i = 0; i < stem->getLeaves().size(); i++) {
		Leaf leaf = *stem->getLeaf(i);
		stem->removeLeaf(i--);

		Vec3 direction = getInitialDirection(leaf, stem, volume);
		Vec3 point = (this->primaryGrowthRate/2.0f) * direction;

		Stem *child = this->plant->addStem(stem);
		child->setDistance(leaf.getPosition());
		child->setSwelling(Vec2(1.5f, 3.0f));
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

		updateBoundingBox(point + stem->getLocation());
	}
}

void Generator::addLeaves(Stem *stem, int node)
{
	Leaf leaf;
	leaf.setMesh(0);
	leaf.setScale(Vec3(0.2f, 0.2f, 0.2f));
	const Path &path = stem->getPath();
	float distance = path.getLength();
	leaf.setPosition(distance);
	LeafData data;
	data.maxForward = 0.0f;
	data.minForward = 0.0f;
	data.localUp = 0.0f;
	data.globalUp = 0.0f;
	leaf.setRotation(data, distance, path, node);
	stem->addLeaf(leaf);
}

Leaf Generator::createLeaf()
{
	Leaf leaf;
	leaf.setScale(Vec3(0.2f, 0.2f, 0.2f));
	leaf.setMesh(0);
	return leaf;
}

/** A bounding box is created to determine how rays should be generated. */
void Generator::updateBoundingBox(Vec3 point)
{
	point.x = std::abs(point.x) * 2.0f + 0.1f;
	point.y = std::abs(point.y) * 2.0f + 0.1f;
	point.z = std::abs(point.z) * 2.0f + 0.1f;
	if (point.x > this->width)
		this->width = point.x;
	if (point.y > this->width)
		this->width = point.y;
	if (point.z > this->width)
		this->width = point.z;
}

void Generator::clearVolume()
{
	this->width = 1.0f;
	this->volume.clear(this->width, this->depth);
}

const Volume *Generator::getVolume()
{
	return &this->volume;
}

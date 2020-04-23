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

#include "patterns.h"
#include "generator.h"
#include <cmath>
#include <limits>

using namespace pg;
using std::cos;
using std::sin;
using std::vector;

#define PI 3.14159265359f

Generator::Generator()
{
	this->rayCount = 200;
	this->rayLevels = 100;
	this->minRadius = 0.001f;
	this->primaryGrowthRate = 0.1f;
	this->secondaryGrowthRate = 0.001f;

	Stem *root = this->plant.createRoot();
	root->setResolution(6);
	
	Path path;
	Spline spline;
	std::vector<Vec3> controls;
	Vec3 control = {};
	controls.push_back(control);
	spline.setControls(controls);
	spline.setDegree(1);
	path.setSpline(spline);
	path.setMinRadius(this->minRadius);
	path.setMaxRadius(this->minRadius);
	path.setRadius(getDefaultCurve(0));
	root->setPath(path);

	Geometry geom;
	geom.setPlane();
	this->plant.addLeafMesh(geom);
	this->leafGeomID = geom.getID();

	updateBoundingBox(control);
}

void Generator::grow(int cycles, int nodes)
{
	for (int i = 0; i < cycles; i++) {
		if (i > 0)
			addStems(this->plant.getRoot());
		for (int j = 0; j < nodes; j++)
			addNodes();
	}
	getRays();
}

void Generator::addStems(Stem *stem)
{
	Stem *child = stem->getChild();
	while (child) {
		addStems(child);
		child = child->getSibling();
	}
	
	std::map<long, Leaf> leaves = stem->getLeaves();
	for (auto it = leaves.begin(), last = --leaves.end(); it != last;) {
		long leafID = it->first;
		Leaf leaf = it->second;
		
		/* TODO: Not all leaves should be removed. */
		it = leaves.erase(it);
		stem->removeLeaf(leafID);
		
		Stem *child = plant.addStem(stem);
		child->setPosition(leaf.getPosition());
		Path path = child->getPath();
		Spline spline = path.getSpline();
		spline.addControl({0.0f, 0.0f, 0.0f});
		spline.setDegree(1);
		path.setSpline(spline);
		path.setMinRadius(this->minRadius);
		path.setMaxRadius(this->minRadius);
		path.setRadius(getDefaultCurve(0));
		child->setPath(path);
		
		Leaf childLeaf = createLeaf();
		childLeaf.setPosition(0.0f);
		child->addLeaf(childLeaf);
	}
}

void Generator::addNodes()
{
	Stem *root = plant.getRoot();
	this->growth.clear();

	vector<Ray> rays = getRays();
	for (Ray ray : rays) {
		Intersection intersection = intersect(root, ray);
		Stem *stem = intersection.stem;
		if (stem) {
			if (this->growth.find(stem) == this->growth.end()) {
				Light growth = {};
				this->growth[stem] = growth;
			}
			this->growth[stem].direction += ray.direction;
			this->growth[stem].rays++;
		}
	}
	
	propagateGrowth(root);
	for (auto &pair : this->growth)
		addNode(pair.first, pair.second);
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

	for (const auto pair : stem->getLeaves()) {
		const Leaf *leaf = &pair.second;
		float radius = leaf->getScale().x * 0.5f;
		float distance = leaf->getPosition();
		Vec3 location = stem->getLocation(); 
		Vec3 direction = {0.0f, 0.0f, 1.0f};
		if (path.getSize() > 1) {
			location += path.getIntermediate(distance);
			direction = path.getIntermediateDirection(distance);
		}
		direction = leaf->getDirection(direction);
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
Generator::Light Generator::propagateGrowth(Stem *stem)
{
	Light growth = {};
	if (this->growth.find(stem) != this->growth.end())
		growth = this->growth.at(stem);

	Stem *child = stem->getChild();
	while (child) {
		Stem *sibling = child->getSibling();
		Light childGrowth = propagateGrowth(child);
		growth.direction += childGrowth.direction;
		growth.rays += childGrowth.rays;
		child = sibling;
	}

	if (growth.rays > 0)
		this->growth[stem] = growth;
	else if (stem->getParent()) {
		Path path = stem->getPath();
		if (path.getSize() > 1) {
			growth.direction = path.getDirection(path.getSize()-1);
			growth.rays = 1;
			this->growth[stem] = growth;	
		} else {
			this->growth.erase(stem);
			delete plant.extractStem(stem);
		}
	} else {
		growth.direction = {0.0f, -1.0f, 0.0f};
		growth.rays = 1;
		this->growth[stem] = growth;
	}
	
	return growth;
}

void Generator::addNode(Stem *stem, Generator::Light growth)
{
	growth.direction /= growth.rays;
	growth.direction = normalize(growth.direction);
	
	Path path = stem->getPath();
	Spline spline = path.getSpline();
	std::vector<Vec3> controls = spline.getControls();

	Vec3 point = controls.back();
	point += -this->primaryGrowthRate * growth.direction;
	controls.push_back(point);
	updateBoundingBox(point + stem->getLocation());
	
	spline.setControls(controls);
	path.setSpline(spline);
	path.setMaxRadius(path.getMaxRadius()+this->secondaryGrowthRate);
	stem->setPath(path);
	
	addLeaves(stem, stem->getPath().getLength());
}

void Generator::addLeaves(Stem *stem, float distance)
{
	Quat rotation = {};
	auto leaves = stem->getLeaves();
	if (leaves.empty())
		rotation.w = 1.0f;
	else {
		Leaf prevLeaf = leaves.rbegin()->second;
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
	leaf.setScale({0.04f, 0.04f, 0.04f});
	leaf.setMesh(this->leafGeomID);
	return leaf;
}

/** Rays are used to estimate how much light each leaf receives. */
vector<Ray> Generator::getRays()
{
	vector<Ray> rays;
	for (int i = 1; i <= this->rayLevels; i++) {
		float angle = i / (float)this->rayLevels * PI * 0.5f;
		int rayCount = (this->rayCount-1) * cos(angle) + 1;
		float y = sin(angle) * this->width;
		float radius = this->width * cos(angle);
		
		for (int j = 0; j < rayCount; j++) {
			float angle = j*(2.0f*PI/rayCount);
			float x = cos(angle) * radius;
			float z = sin(angle) * radius;
			Vec3 origin = {-x, 0.0f, -z};
			Ray ray;
			ray.origin = {x, y, z};
			ray.direction = normalize(origin-ray.origin);
			rays.push_back(ray);
		}
	}
	return rays;
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

Plant *Generator::getPlant()
{
	return &this->plant;
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
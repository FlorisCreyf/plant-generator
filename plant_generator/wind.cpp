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

#include "wind.h"

const float pi = 3.14159265359f;

using namespace pg;
using std::vector;

Wind::Wind() :
	direction(1.0f, 0.0f, 0.0f),
	speed(1.0f),
	timeStep(30),
	frameCount(21),
	seed(0),
	randomGenerator(0)
{

}

void Wind::setSeed(int seed)
{
	this->seed = seed;
}

void Wind::setFrameCount(int count)
{
	this->frameCount = count;
}

int Wind::getFrameCount() const
{
	return this->frameCount;
}

void Wind::setTimeStep(int step)
{
	this->timeStep = step;
}

int Wind::getTimeStep() const
{
	return this->timeStep;
}

int Wind::getDuration() const
{
	return (this->frameCount-1) * this->timeStep;
}

void Wind::setSpeed(float speed)
{
	this->speed = speed;
}

void Wind::setDirection(Vec3 direction)
{
	this->direction = direction;
}

Animation Wind::generate(Plant *plant)
{
	Animation animation;
	Stem *root = plant->getRoot();
	if (!root || this->speed <= 0.0f)
		return animation;

	this->randomGenerator.seed(this->seed);
	animation.timeStep = this->timeStep;
	root->clearJoints();
	size_t count = 0;
	generateJoint(root, -1, -1, count);
	animation.frames.clear();
	animation.frames.resize(count, vector<KeyFrame>(this->frameCount));
	transformJoint(plant, root, root->getLocation(), animation);
	return animation;
}

void Wind::setRotation(int joint, float distance, float radius, Vec3 direction,
	Animation &animation)
{
	radius += 1.0f;
	float resistance = radius*radius;
	float intensity = 0.1f * (distance*this->speed) / resistance;
	Vec3 orthogonalDirection = cross(this->direction, direction);
	std::uniform_real_distribution<float> dis(0.0f, pi);
	float offset = dis(this->randomGenerator);

	for (int i = 0; i < this->frameCount; i++) {
		KeyFrame &frame = animation.frames[joint][i];

		float x = i * 2.0f*pi/(this->frameCount-1) + offset;
		float wave = sin(x) * cos(2.0f*x+pi*0.25f);
		float t = intensity * wave;
		Vec3 movement = lerp(direction, this->direction, t);
		frame.rotation = rotateIntoVecQ(direction, movement);

		wave = sin(x+pi*0.5f) * cos(2.0f*x+pi*0.25f);
		t = intensity * wave;
		movement = lerp(direction, orthogonalDirection, t);
		frame.rotation *= rotateIntoVecQ(direction, movement);
	}
}

void Wind::setNoRotation(int joint, Animation &animation)
{
	for (int i = 0; i < this->frameCount; i++) {
		KeyFrame &frame = animation.frames[joint][i];
		frame.rotation = Quat(0.0f, 0.0f, 0.0f, 1.0f);
	}
}

void Wind::setTranslation(int joint, Vec3 point, Vec3 origin,
	Animation &animation)
{
	size_t size = animation.frames[joint].size();
	for (size_t i = 0; i < size; i++) {
		KeyFrame &frame = animation.frames[joint][i];
		frame.translation = toVec4(point - origin, 0.0f);
	}
}

void Wind::setRootTranslation(Stem *root, Animation &animation)
{
	size_t size = animation.frames[0].size();
	for (size_t i = 0; i < size; i++) {
		KeyFrame &frame = animation.frames[0][i];
		frame.translation = toVec4(root->getLocation(), 0.0f);
	}
}

void Wind::transformChildJoint(Stem *child, unsigned parentID, Plant *plant,
	Vec3 location, Animation &animation)
{
	if (child->hasJoints()) {
		Joint joint = child->getJoints()[0];
		if (joint.getParentID() == parentID)
			transformJoint(plant, child, location, animation);
	}
}

void Wind::transformJoint(Plant *plant, Stem *stem, Vec3 prevLocation,
	Animation &animation)
{
	Path path = stem->getPath();
	vector<Joint> joints = stem->getJoints();

	for (size_t i = 0; i < joints.size(); i++) {
		Joint joint = joints[i];
		size_t index = joint.getPathIndex();
		Vec3 location = stem->getLocation() + path.get(index);

		if (i > 0) {
			float distance = path.getDistance(index-1, index);
			float radius = plant->getRadius(stem, i);
			Vec3 direction = path.getDirection(index);
			setRotation(joint.getID(), distance, radius, direction,
				animation);
		} else
			setNoRotation(joint.getID(), animation);

		if (i > 0 || stem->getParent())
			setTranslation(joint.getID(), location, prevLocation,
				animation);
		else
			setRootTranslation(stem, animation);

		Stem *child = stem->getChild();
		while (child) {
			transformChildJoint(child, joint.getID(), plant,
				location, animation);
			child = child->getSibling();
		}

		prevLocation = path.get(index) + stem->getLocation();
	}
}

int Wind::generateJoint(Stem *stem, int id, int parentID, size_t &count)
{
	float distance = 0.0f;
	Path path = stem->getPath();
	Spline spline = path.getSpline();
	int degree = spline.getDegree();
	std::vector<Vec3> controls = spline.getControls();

	size_t controlCount = controls.size();
	size_t i = stem->getParent() ? degree : 0;
	for (; i < controlCount-1; i += degree) {
		size_t start = path.toPathIndex(i);
		size_t end = path.toPathIndex(i + degree);
		distance += path.getDistance(start, end);

		stem->addJoint(Joint(++id, parentID, start));
		parentID = id;
		count++;

		Stem *child = stem->getChild();
		while (child) {
			bool needsJoints = child->getJoints().empty();
			if (child->getDistance() < distance && needsJoints)
				id = generateJoint(child, id, parentID, count);
			child = child->getSibling();
		}
	}
	return id;
}

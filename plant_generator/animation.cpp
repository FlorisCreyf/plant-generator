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

#include "animation.h"

using namespace pg;

std::vector<KeyFrame> Animation::getFrame(int ticks, Stem *stem)
{
	this->mixedFrames.resize(this->frames.size());
	size_t index1 = ticks / this->timeStep;
	size_t index2 = index1 + 1;
	float t = (ticks % this->timeStep) / static_cast<float>(this->timeStep);
	createFrame(t, index1, index2, stem);
	return this->mixedFrames;
}

void Animation::createFrame(float t, size_t index1, size_t index2, Stem *stem)
{
	const std::vector<Joint> joints = stem->getJoints();
	for (const Joint &joint : joints) {
		size_t jointIndex = joint.getID();
		size_t parentJointIndex = joint.getParentID();
		KeyFrame &frame1 = this->frames[jointIndex][index1];
		KeyFrame &frame2 = this->frames[jointIndex][index2];
		KeyFrame &frame = this->mixedFrames[jointIndex];
		Quat rotation = nlerp(frame1.rotation, frame2.rotation, t);

		if (joint.getID() > 0) {
			KeyFrame &prev = this->mixedFrames[parentJointIndex];

			frame.translation = prev.translation;
			frame.translation += frame1.translation;
			frame.rotation = rotation * prev.rotation;

			Quat quat = toQuat(frame1.translation);
			quat = prev.rotation * quat * conjugate(prev.rotation);
			frame.finalTranslation = toVec4(quat);
			frame.finalTranslation += prev.finalTranslation;
		} else {
			frame.rotation = rotation;
			frame.translation = frame1.translation;
			frame.finalTranslation = frame1.translation;
		}
	}

	Stem *child = stem->getChild();
	while (child) {
		createFrame(t, index1, index2, child);
		child = child->getSibling();
	}
}

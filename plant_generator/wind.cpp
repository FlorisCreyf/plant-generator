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

using namespace pg;

Wind::Wind(Plant *plant)
{
	this->plant = plant;
}

void Wind::generate()
{
	this->plant->getRoot()->clearJoints();
	generate(this->plant->getRoot(), -1, -1);
}

int Wind::generate(Stem *stem, int id, int parentID)
{
	Path path = stem->getPath();
	Spline spline = path.getSpline();
	std::vector<Vec3> controls = spline.getControls();
	size_t size = controls.size();
	int degree = spline.getDegree();
	float distance = 0.0f;

	for (size_t i = stem->getParent() ? 1 : 0; i < size-1; i += degree) {
		size_t start = path.toPathIndex(i);
		size_t end = path.toPathIndex(i + degree);
		distance += path.getDistance(start, end);

		stem->addJoint(Joint(++id, parentID, start));
		parentID = id;

		Stem *child = stem->getChild();
		while (child) {
			bool needsJoints = child->getJoints().empty();
			if (child->getPosition() < distance && needsJoints)
				id = generate(child, id, parentID);
			child = child->getSibling();
		}
	}
	return id;
}

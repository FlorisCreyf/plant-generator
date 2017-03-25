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

#include "stem_modifier.h"
#include "tree_impl.h"

StemModifier::StemModifier(ProcGenerator *pg)
{
	this->pg = pg;
}

void StemModifier::distribute(Stem *stem, float length)
{
	float newLength = stem->getPath().getLength() - stem->baseLength;
	for (size_t i = 0; i < stem->getChildCount(); i++) {
		Stem *child = stem->getChild(i);
		float t = (child->getPosition() - stem->baseLength) / length;
		child->setPosition(newLength * t + stem->baseLength);
	}
}

void StemModifier::updateStemDensity(Stem *stem)
{
	if (stem->stemDensity > 0.0f && stem->mode == Flags::ASSISTED) {
		float density = stem->stemDensity;
		stem->removeLateralStems();
		stem->stemDensity = density;
		pg->addLateralStems(stem, stem->baseLength);
	} else if (stem->stemDensity == 0.0f) {
		stem->removeLateralStems();
	}
}

void StemModifier::updateBaseLength(Stem *stem)
{
	if (stem->stemDensity == 0.0f)
		return;
	
	if (stem->mode == Flags::MANUAL) {
		float distance = 1.0f / stem->stemDensity;
		float position = stem->baseLength;
		for (size_t i = 0; i < stem->getChildCount(); i++) {
			Stem *child = stem->getChild(i);
			child->setPosition(position);
			position += distance;
		}
	} else if (stem->mode == Flags::ASSISTED) {
		updateStemDensity(stem);
	}
}
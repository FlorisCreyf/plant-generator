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

#ifndef TM_STEM_MODIFIER_H
#define TM_STEM_MODIFIER_H

#include "procedural_generator.h"

class StemModifier {
public:
	StemModifier(ProcGenerator *pg);
	void distribute(Stem *stem, float length);
	void updateStemDensity(Stem *stem);
	void updateBaseLength(Stem *stem);
	
private:
	ProcGenerator *pg;
};

#endif /* TM_STEM_MODIFIER_H */


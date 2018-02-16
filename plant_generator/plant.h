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

#ifndef PG_PLANT_H
#define PG_PLANT_H

#include "stem.h"
#include <vector>
#include <boost/archive/text_oarchive.hpp>

namespace pg {
	class Plant {
		friend class boost::serialization::access;

		Stem *root;

		template<class Archive>
		void serialize(Archive &ar, const unsigned int version)
		{
			(void)version;
			ar & root;
		}

	public:
		Plant();
		~Plant();

		Stem *getRoot();
		Stem *addStem(Stem *stem);
		void removeStem(Stem *stem);
		Stem *copy(Stem *stem, Stem *parent, Stem **ref = nullptr);
		/** Removes the root and sets root to nullptr. */
		void removeRoot();
		/** This does not delete the previous root. */
		void setRoot(Stem *stem);
	};
}

#endif /* PG_PLANT_H */
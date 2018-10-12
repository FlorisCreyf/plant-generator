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

		bool contains(pg::Stem *a, pg::Stem *b);
		void removeFromTree(pg::Stem *stem);

	public:
		Plant();
		~Plant();

		/**
		 * This method needs to be called some time after calling
		 * release() to free up memory.
		 */
		static void deleteStem(pg::Stem *stem);

		Stem *getRoot();
		Stem *addStem(Stem *stem);
		void removeStem(Stem *stem);
		void insert(Stem *parent, Stem *child);
		/** Remove without deletion.*/
		void release(Stem *stem);
		bool contains(pg::Stem *stem);
		/** Removes the root and sets root to nullptr. */
		void removeRoot();
		/** This does not delete the previous root. */
		void setRoot(Stem * stem);
	};
}

#endif /* PG_PLANT_H */

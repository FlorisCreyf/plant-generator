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
#include "material.h"
#include "geometry.h"
#include "stem_pool.h"
#include <map>

#ifdef PG_SERIALIZE
#include <boost/serialization/map.hpp>
#include <boost/archive/text_oarchive.hpp>
#endif

namespace pg {
	class Plant {
	public:
		Plant();
		~Plant();
		StemPool *getStemPool();

		/** Add a new stem to the plant at a parent stem. */
		Stem *addStem(Stem *parent);
		/** Remove all stems from the plant and create a new root. */
		Stem *createRoot();
		/** Delete a stem. */
		void deleteStem(Stem *);
		/** Return the root or trunk of the plant. */
		Stem *getRoot();
		const Stem *getRoot() const;
		/** Remove all stems in the plant. */
		void removeRoot();

		struct Extraction {
			Stem *address;
			Stem *parent;
			Stem value;
		};
		/** Remove a stem and its descendants from the plant. */
		void extractStems(Stem *stem, std::vector<Extraction> &stems);
		/** Reinsert extracted stems. */
		void reinsertStems(std::vector<Extraction> &stem);

		void addMaterial(Material material);
		void removeMaterial(long id);
		Material getMaterial(long id) const;
		std::map<long, Material> getMaterials() const;

		void addLeafMesh(Geometry mesh);
		void removeLeafMesh(long id);
		void removeLeafMeshes();
		Geometry getLeafMesh(long id) const;
		std::map<long, Geometry> getLeafMeshes() const;

	private:
		Stem *root;
		std::map<long, Material> materials;
		std::map<long, Geometry> leafMeshes;
		StemPool stemPool;

		void removeMaterial(Stem *, long);
		void updateDepth(Stem *, int);
		void deallocateStems(Stem *);
		void insertStem(Stem *, Stem *);
		void decouple(Stem *);
		Stem *move(Stem *);
		void copy(std::vector<Extraction> &, Stem *);

		#ifdef PG_SERIALIZE
		friend class boost::serialization::access;
		template<class Archive>
		void save(Archive &ar, const unsigned int version) const
		{
			(void)version;
			ar & root;
			ar & materials;
			ar & leafMeshes;
		}
		template<class Archive>
		void load(Archive &ar, const unsigned int version)
		{
			(void)version;
			ar & root;
			root = move(root);
			ar & materials;
			ar & leafMeshes;
		}
		BOOST_SERIALIZATION_SPLIT_MEMBER()
		#endif
	};
}

#endif /* PG_PLANT_H */

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

#include "curve.h"
#include "geometry.h"
#include "material.h"
#include "stem.h"
#include "stem_pool.h"
#include <vector>

#ifdef PG_SERIALIZE
#include <boost/archive/text_oarchive.hpp>
#endif

namespace pg {
	class Plant {
	public:
		Plant();
		Plant(const Plant &original) = delete;
		Plant &operator=(const Plant &original) = delete;
		~Plant();

		/** Initialize the plant with default objects. */
		void setDefault();
		StemPool *getStemPool();

		/** Add a new stem to the plant at a parent stem. */
		Stem *addStem(Stem *parent);
		/** Remove all stems from the plant and create a new root. */
		Stem *createRoot();
		/** Delete a stem. */
		void deleteStem(Stem *stem);
		/** Return the root or trunk of the plant. */
		Stem *getRoot();
		const Stem *getRoot() const;
		/** Remove all stems in the plant. */
		void removeRoot();
		/** Remove all resources. */
		void erase();

		/* Remove a stem that has no children. */
		Stem extractStem(Stem *stem);
		/** Remove a stem and its descendants from the plant. */
		void extractStems(Stem *stem, std::vector<Stem> &stems);
		/** Reinsert a single extracted stem. */
		void reinsertStem(Stem &stem);
		/** Reinsert extracted stems. */
		void reinsertStems(std::vector<Stem> &stem);

		float getRadius(Stem *stem, unsigned index) const;
		float getIntermediateRadius(Stem *stem, float t) const;

		void addCurve(Curve curve);
		void updateCurve(Curve curve, unsigned index);
		void removeCurve(unsigned index);
		Curve getCurve(unsigned index) const;
		const std::vector<Curve> &getCurves() const;

		void addMaterial(Material material);
		void updateMaterial(Material material, unsigned index);
		void removeMaterial(unsigned index);
		Material getMaterial(unsigned index) const;
		const std::vector<Material> &getMaterials() const;

		void addLeafMesh(Geometry mesh);
		void updateLeafMesh(Geometry mesh, unsigned index);
		void removeLeafMesh(unsigned index);
		Geometry getLeafMesh(unsigned index) const;
		const std::vector<Geometry> &getLeafMeshes() const;

	private:
		Stem *root;
		std::vector<Material> materials;
		std::vector<Geometry> leafMeshes;
		std::vector<Curve> curves;
		StemPool stemPool;

		void removeCurve(Stem *, unsigned);
		void removeMaterial(Stem *, unsigned);
		void removeLeafMesh(Stem *, unsigned);

		void deallocateStems(Stem *);
		void insertStem(Stem *, Stem *, Stem *);
		void insertStemAfterSibling(Stem *, Stem *, Stem *);
		void insertStemBeforeSibling(Stem *, Stem *, Stem *);
		void insertStemAtBeginning(Stem *, Stem *);
		Stem *getLastSibling(Stem *);
		void decouple(Stem *);
		Stem *move(Stem *);
		void copy(std::vector<Stem> &, Stem *);

#ifdef PG_SERIALIZE
		friend class boost::serialization::access;
		template<class Archive>
		void save(Archive &ar, const unsigned) const
		{
			ar & root;
			ar & materials;
			ar & leafMeshes;
			ar & curves;
		}
		template<class Archive>
		void load(Archive &ar, const unsigned)
		{
			ar & root;
			root = move(root);
			ar & materials;
			ar & leafMeshes;
			ar & curves;
		}
		BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif
	};
}

#endif

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

#ifndef PG_DERIVATION_H
#define PG_DERIVATION_H

#include "spline.h"
#include <string>
#include <vector>

#ifdef PG_SERIALIZE
#include <boost/archive/text_oarchive.hpp>
#endif

namespace pg {
	struct Derivation {
		Spline stemDensityCurve;
		Spline leafDensityCurve;
		Vec3 leafScale = Vec3(1.0f, 1.0f, 1.0f);
		float stemDensity = 0.0f;
		float leafDensity = 0.0f;
		float stemStart = 0.0f;
		float leafDistance = 2.0f;
		float lengthFactor = 1.0f;
		float radiusThreshold = 0.0f;
		float leafRotation = 0.0f;
		float minUp = 0.0f;
		float maxUp = 1.0f;
		float minDirection = 0.0f;
		float maxDirection = 1.0f;
		int leavesPerNode = 1;

		#ifdef PG_SERIALIZE
		template<class Archive>
		void serialize(Archive &ar, const unsigned version)
		{
			ar & stemDensity;
			ar & leafDensity;
			ar & stemDensityCurve;
			ar & leafDensityCurve;
			ar & stemStart;
			ar & leafDistance;
			ar & radiusThreshold;
			ar & lengthFactor;
			if (version < 2) {
				int arrangement;
				ar & arrangement;
			}
			if (version >= 1)
				ar & leafScale;
			if (version == 2) {
				ar & leafRotation;
				ar & leavesPerNode;
				ar & minUp;
				ar & maxUp;
				ar & minDirection;
				ar & maxDirection;
			}
		}
		#endif /* PG_SERIALIZE */
	};

	class DerivationNode {
		friend class DerivationTree;

		DerivationNode *child = nullptr;
		DerivationNode *parent = nullptr;
		DerivationNode *nextSibling = nullptr;
		DerivationNode *prevSibling = nullptr;
		Derivation data;

		#ifdef PG_SERIALIZE
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive &ar, const unsigned)
		{
			ar & child;
			ar & parent;
			ar & nextSibling;
			ar & prevSibling;
			ar & data;
		}
		#endif /* PG_SERIALIZE */

	public:
		Derivation getData() const;
		void setData(Derivation data);
		const DerivationNode *getChild() const;
		const DerivationNode *getSibling() const;
		const DerivationNode *getNextSibling() const;
		const DerivationNode *getPrevSibling() const;
		const DerivationNode *getParent() const;
	};

	class DerivationTree {
	public:
		DerivationTree();
		~DerivationTree();
		DerivationTree(const DerivationTree &original);
		DerivationTree &operator=(const DerivationTree &derivation);
		void setSeed(int seed);
		int getSeed() const;
		void reset();
		DerivationNode *getRoot() const;
		DerivationNode *createRoot();
		DerivationNode *addChild(std::string name);
		DerivationNode *addSibling(std::string name);
		DerivationNode *get(std::string name) const;
		bool remove(std::string name);
		std::vector<std::string> getNames() const;

	private:
		unsigned seed = 0;
		DerivationNode *root;

		void copy(const DerivationTree &);
		void removeChildNode(DerivationNode *node);
		void copyNode(const DerivationNode *, DerivationNode *);
		void getNames(std::vector<std::string> &, std::string,
			DerivationNode *) const;
		DerivationNode *getNode(const std::string &, size_t,
			DerivationNode *) const;
		int getSize(const std::string &, size_t &) const;

		#ifdef PG_SERIALIZE
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive &ar, const unsigned)
		{
			ar & seed;
			ar & root;
		}
		#endif /* PG_SERIALIZE */
	};
}

#ifdef PG_SERIALIZE
BOOST_CLASS_VERSION(pg::Derivation, 2);
#endif

#endif /* PG_DERIVATION_H */

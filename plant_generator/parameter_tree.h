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

#ifndef PG_PARAMETER_TREE_H
#define PG_PARAMETER_TREE_H

#include "spline.h"
#include <string>
#include <vector>

#ifdef PG_SERIALIZE
#include <boost/archive/text_oarchive.hpp>
#endif

namespace pg {
	struct LeafData {
		Spline densityCurve;
		Vec3 scale = Vec3(1.0f, 1.0f, 1.0f);
		float density = 0.0f;
		float distance = 2.0f;
		float rotation = 3.14159265359f;
		float minUp = 0.0f;
		float maxUp = 1.0f;
		float minDirection = 0.0f;
		float maxDirection = 1.0f;
		int leavesPerNode = 1;

	private:
		#ifdef PG_SERIALIZE
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive &ar, const unsigned)
		{
			ar & densityCurve;
			ar & scale;
			ar & density;
			ar & distance;
			ar & rotation;
			ar & minUp;
			ar & maxUp;
			ar & minDirection;
			ar & maxDirection;
			ar & leavesPerNode;
		}
		#endif /* PG_SERIALIZE */
	};

	struct StemData {
		Spline densityCurve;
		float density = 0.0f;
		float start = 0.0f;
		float angleVariation = 0.5f;
		float length = 1.0f;
		float radiusThreshold = 0.0f;
		float scale = 1.0f;
		LeafData leaf;

	private:
		#ifdef PG_SERIALIZE
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive &ar, const unsigned)
		{
			ar & leaf;
			ar & densityCurve;
			ar & density;
			ar & start;
			ar & length;
			ar & radiusThreshold;
			ar & angleVariation;
			ar & scale;
		}
		#endif /* PG_SERIALIZE */
	};

	class ParameterNode {
		friend class ParameterTree;

		ParameterNode *child = nullptr;
		ParameterNode *parent = nullptr;
		ParameterNode *nextSibling = nullptr;
		ParameterNode *prevSibling = nullptr;
		StemData data;

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
		StemData getData() const;
		void setData(StemData data);
		const ParameterNode *getChild() const;
		const ParameterNode *getSibling() const;
		const ParameterNode *getNextSibling() const;
		const ParameterNode *getPrevSibling() const;
		const ParameterNode *getParent() const;
	};

	class ParameterRoot {
		friend class ParameterTree;

		unsigned seed = 0;
		ParameterNode *node = nullptr;

		#ifdef PG_SERIALIZE
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive &ar, const unsigned)
		{
			ar & seed;
			ar & node;
		}
		#endif /* PG_SERIALIZE */

	public:
		void setSeed(int seed);
		int getSeed() const;
		ParameterNode *getNode() const;
	};

	class ParameterTree {
		ParameterRoot *root;

		void copy(const ParameterTree &);
		void removeChildNode(ParameterNode *);
		void copyNode(const ParameterNode *, ParameterNode *);
		void getNames(std::vector<std::string> &, std::string,
			ParameterNode *) const;
		ParameterNode *getNode(const std::string &, size_t,
			ParameterNode *) const;
		int getSize(const std::string &, size_t &) const;

		#ifdef PG_SERIALIZE
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive &ar, const unsigned)
		{
			ar & root;
		}
		#endif /* PG_SERIALIZE */

	public:
		ParameterTree();
		~ParameterTree();
		ParameterTree(const ParameterTree &original);
		ParameterTree &operator=(const ParameterTree &derivation);
		void reset();
		ParameterRoot *getRoot() const;
		ParameterRoot *createRoot();
		ParameterNode *addChild(std::string name);
		ParameterNode *addSibling(std::string name);
		ParameterNode *get(std::string name) const;
		bool remove(std::string name);
		std::vector<std::string> getNames() const;
	};
}

#endif /* PG_PARAMETER_TREE_H */

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
		Vec3 scale;
		float density;
		float distance;
		float rotation;
		float minUp;
		float maxUp;
		float minDirection;
		float maxDirection;
		int leavesPerNode;

		LeafData();

	private:
#ifdef PG_SERIALIZE
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive &ar, const unsigned)
		{
			ar & densityCurve;
			ar & density;
			ar & distance;
			ar & rotation;
			ar & minUp;
			ar & maxUp;
			ar & minDirection;
			ar & maxDirection;
			ar & leavesPerNode;
			ar & scale;
		}
#endif
	};

	struct StemData {
		Spline densityCurve;
		float density;
		float start;
		float length;
		float angleVariation;
		float radiusThreshold;
		float scale;
		LeafData leaf;

		StemData();

	private:
#ifdef PG_SERIALIZE
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive &ar, const unsigned)
		{
			ar & densityCurve;
			ar & density;
			ar & start;
			ar & length;
			ar & radiusThreshold;
			ar & angleVariation;
			ar & scale;
			ar & leaf;
		}
#endif
	};

	class ParameterNode {
		friend class ParameterTree;

		ParameterNode *child;
		ParameterNode *parent;
		ParameterNode *nextSibling;
		ParameterNode *prevSibling;
		StemData data;

		ParameterNode();
		ParameterNode(const ParameterNode &) = delete;
		ParameterNode &operator=(const ParameterNode &) = delete;

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
#endif
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

		unsigned seed;
		ParameterNode *node;

#ifdef PG_SERIALIZE
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive &ar, const unsigned)
		{
			ar & seed;
			ar & node;
		}
#endif

	public:
		ParameterRoot();
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
#endif

	public:
		ParameterTree();
		~ParameterTree();
		ParameterTree(const ParameterTree &original);
		ParameterTree &operator=(const ParameterTree &derivation);
		void reset();
		ParameterRoot *getRoot() const;
		ParameterRoot *createRoot();
		ParameterNode *getNode() const;
		ParameterNode *addChild(std::string name);
		ParameterNode *addSibling(std::string name);
		ParameterNode *get(std::string name) const;
		bool remove(std::string name);
		std::vector<std::string> getNames() const;
	};
}

#endif

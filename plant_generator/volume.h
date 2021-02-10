/* Copyright 2021 Floris Creyf
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

#ifndef PG_VOLUME_H
#define PG_VOLUME_H

#include "math/intersection.h"
#include "math/vec3.h"

namespace pg {
	class Volume {
	public:
		class Node {
			Node *nodes;
			Node *parent;
			Vec3 center;
			float size;
			float weight;
			int depth;

			Node();
			Node *getAdjacentNode(Node *, int, Vec3, bool);

		public:
			~Node();
			Node(Vec3 center, float size);
			Node *getParent();
			Node *getNode(int index);
			Node *getNextNode(Node *, int, Vec3);
			Node *getPreviousNode(Node *, int, Vec3);
			Node *getChildNode(Node *, Vec3);
			Node *getAdjacentNode(Node *node, Ray ray);
			void divide();
			void setWeight(float weight);
			float getWeight() const;
			Vec3 getCenter() const;
			float getSize() const;
			int getDepth() const;
		};

		Volume(float size, int depth);
		Node *addNode(Vec3 point);
		void addLine(Vec3 a, Vec3 b, float weight);
		Node *getNode(Vec3 point);
		Node *getRoot();

	private:
		const float size;
		const int depth;
		Node root;

		Node *getNode(Vec3 point, Node *node);
	};
}

#endif
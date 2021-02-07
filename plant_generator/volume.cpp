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

#include "volume.h"

using pg::Ray;
using pg::Vec3;
using pg::Volume;

typedef Volume::Node Node;

Volume::Volume(float size, int depth) :
	size(size),
	depth(depth),
	root(Vec3(0.0f, size*0.5f, 0.0f), 0.5f*size)
{

}

Node *Volume::addNode(Vec3 point)
{
	Node *node = &this->root;
	while (node->getDepth() < this->depth) {
		Node *cnode = getNode(point, node);
		if (cnode == node)
			node->divide();
		node = getNode(point, node);
	}
	return node;
}

void Volume::addLine(Vec3 a, Vec3 b, float weight)
{
	Ray ray(a, normalize(b-a));
	Node *node = addNode(a);
	Node *lastNode = addNode(b);
	node->setWeight(weight);
	while (node != lastNode) {
		Node *nextNode = node->getAdjacentNode(node, ray);
		if (nextNode == node)
			break;
		node = nextNode;

		Vec3 center = node->getCenter();
		while (node->getDepth() < this->depth) {
			node->divide();
			node = node->getChildNode(node, center);
		}
		node->setWeight(weight);
	}
}

Node *Node::getAdjacentNode(Volume::Node *node, Ray ray)
{
	Vec3 d;
	Plane plane;
	plane.point = node->getCenter();
	if (ray.direction.x < 0.0f) {
		plane.point.x -= node->getSize();
		d.x = -1.0f;
	} else {
		plane.point.x += node->getSize();
		d.x = 1.0f;
	}
	if (ray.direction.y < 0.0f) {
		plane.point.y -= node->getSize();
		d.y = -1.0f;
	} else {
		plane.point.y += node->getSize();
		d.y = 1.0f;
	}
	if (ray.direction.z < 0.0f) {
		plane.point.z -= node->getSize();
		d.z = -1.0f;
	} else {
		plane.point.z += node->getSize();
		d.z = 1.0f;
	}

	plane.normal = Vec3(d.x, 0.0f, 0.0f);
	float t = intersectsPlane(ray, plane);
	Vec3 p = t*ray.direction + ray.origin;
	if (t && d.y*p.y <= d.y*plane.point.y && d.z*p.z <= d.z*plane.point.z)
		node = getAdjacentNode(node, 1, p, d.x > 0.0f);

	plane.normal = Vec3(0.0f, d.y, 0.0f);
	t = intersectsPlane(ray, plane);
	p = t*ray.direction + ray.origin;
	if (t && d.x*p.x <= d.x*plane.point.x && d.z*p.z <= d.z*plane.point.z)
		node = getAdjacentNode(node, 2, p, d.y > 0.0f);

	plane.normal = Vec3(0.0f, 0.0f, d.z);
	t = intersectsPlane(ray, plane);
	p = t*ray.direction + ray.origin;
	if (t && d.y*p.y <= d.y*plane.point.y && d.x*p.x <= d.x*plane.point.x)
		node = getAdjacentNode(node, 4, p, d.z > 0.0f);

	return node;
}

Node *Node::getAdjacentNode(Node *node, int axis, Vec3 point, bool positive)
{
	if (!node->getParent())
		return node;
	else if (positive)
		return getNextNode(node, axis, point);
	else
		return getPreviousNode(node, axis, point);
}

Node *Node::getNextNode(Node *node, int axis, Vec3 point)
{
	Node *pnode = node->getParent();
	/* A set bit is in the positive direction, and an unset bit is in the
	negative direction (x=1, y=2, z=4). */
	int index = node - pnode->getNode(0);
	if ((index & axis) == axis)
		node = getNextNode(pnode, axis, point);
	else
		node = pnode->getNode(index + axis);
	if (node->getNode(0))
		node = getChildNode(node, point);
	return node;
}

Node *Node::getPreviousNode(Node *node, int axis, Vec3 point)
{
	Node *pnode = node->getParent();
	int index = node - pnode->getNode(0);
	if ((index & axis) == 0)
		node = getPreviousNode(pnode, axis, point);
	else
		node = pnode->getNode(index - axis);
	if (node->getNode(0))
		node = getChildNode(node, point);
	return node;
}

Node *Node::getChildNode(Node *pnode, Vec3 point)
{
	Node *node = nullptr;
	float t = std::numeric_limits<float>::max();
	for (int i = 0; i < 8; i++) {
		Node *cnode = pnode->getNode(i);
		Vec3 d = cnode->getCenter() - point;
		float m = d.x*d.x + d.y*d.y + d.z*d.z;
		if (m < t) {
			t = m;
			node = cnode;
		}
	}
	if (node->getNode(0))
		return getChildNode(node, point);
	else
		return node;
}

Node *Volume::getRoot()
{
	return &this->root;
}

Node *Volume::getNode(Vec3 point)
{
	return getNode(point, &this->root);
}

Node *Volume::getNode(Vec3 point, Node *node)
{
	Node *n = node;
	while (n) {
		Vec3 a = point - node->getCenter();
		int x = a.x >= 0.0f ? 1 : 0;
		int y = a.y >= 0.0f ? 1 : 0;
		int z = a.z >= 0.0f ? 1 : 0;
		n = node->getNode(x + 2*y + 4*z);
		node = n ? n : node;
	}
	return node;
}

Node::Node(Vec3 center, float size) :
	nodes(nullptr),
	parent(nullptr),
	center(center),
	size(size),
	weight(1.0f),
	depth(0)
{

}

Node::Node() : nodes(nullptr), parent(nullptr), weight(1.0f), depth(0)
{

}

Node::~Node()
{
	if (this->nodes)
		delete[] this->nodes;
}

void Node::setWeight(float weight)
{
	this->weight = weight;
}

float Node::getWeight() const
{
	return this->weight;
}

Vec3 Node::getCenter() const
{
	return this->center;
}

float Node::getSize() const
{
	return this->size;
}

int Node::getDepth() const
{
	return this->depth;
}

Node *Node::getParent()
{
	return this->parent;
}

Node *Node::getNode(int index)
{
	if (this->nodes)
		return &this->nodes[index];
	else
		return nullptr;
}

void Node::divide()
{
	this->nodes = new Node[8];
	for (int i = 0; i < 8; i++) {
		Vec3 center = this->center;
		float size = 0.5f * this->size;
		if ((i & 1) == 1)
			center.x += size;
		else
			center.x -= size;
		if ((i & 2) == 2)
			center.y += size;
		else
			center.y -= size;
		if ((i & 4) == 4)
			center.z += size;
		else
			center.z -= size;
		this->nodes[i].center = center;
		this->nodes[i].size = size;
		this->nodes[i].depth = this->depth + 1;
		this->nodes[i].parent = this;
		this->nodes[i].nodes = nullptr;
	}
}

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
	root(Vec3(0.0f, 0.0f, size*0.5f), 0.5f*size)
{

}

void Volume::clear(float size, int depth)
{
	this->size = size;
	this->depth = depth;
	this->root = Node(Vec3(0.0f, 0.0f, size*0.5f), 0.5f*size);
	if (this->root.getNode(0))
		this->root.clear();
}

Node *Volume::addNode(Vec3 point, int depth)
{
	Node *node = getNode(point, &this->root);
	while (node->getDepth() < this->depth && node->getDepth() < depth) {
		node->divide();
		node = getNode(point, node);
	}
	return node;
}

void Volume::addLine(Vec3 a, Vec3 b, float weight, float radius)
{
	float length = magnitude(b-a);
	int depth = std::abs(std::log2(radius/this->size))-1;
	Node *firstNode = addNode(a, depth);
	Node *lastNode = addNode(b, depth);
	a = firstNode->getCenter();
	b = lastNode->getCenter();
	Ray ray(a, normalize(b-a));
	Node *node = firstNode;
	node->setDensity(weight);

	while (node != lastNode) {
		Node *nextNode = node->getAdjacentNode(ray, depth);
		if (!nextNode || nextNode == node)
			break;

		Vec3 center = node->getCenter();
		node = nextNode;
		if (magnitude(a-center) > length)
			break;

		int d = node->getDepth();
		while (d < this->depth && d < depth) {
			node->divide();
			node = node->getChildNode(center, depth);
			d = node->getDepth();
		}
		node->setDensity(weight);
	}
}

Node *Node::getAdjacentNode(Ray ray, int depth)
{
	Vec3 d;
	Plane plane;
	plane.point = this->center;
	if (ray.direction.x < 0.0f) {
		plane.point.x -= this->size;
		d.x = -1.0f;
	} else {
		plane.point.x += this->size;
		d.x = 1.0f;
	}
	if (ray.direction.y < 0.0f) {
		plane.point.y -= this->size;
		d.y = -1.0f;
	} else {
		plane.point.y += this->size;
		d.y = 1.0f;
	}
	if (ray.direction.z < 0.0f) {
		plane.point.z -= this->size;
		d.z = -1.0f;
	} else {
		plane.point.z += this->size;
		d.z = 1.0f;
	}

	Node *node = this;
	Vec3 p;
	float t;

	plane.normal = Vec3(d.x, 0.0f, 0.0f);
	t = intersectsPlane(ray, plane);
	p = t*ray.direction + ray.origin;
	p.y -= 0.000001f;
	p.z -= 0.000001f;
	if (t && d.y*p.y < d.y*plane.point.y && d.z*p.z < d.z*plane.point.z)
		node = node->getAdjacentNode(1, p, d.x > 0.0f, depth);

	plane.normal = Vec3(0.0f, d.y, 0.0f);
	t = intersectsPlane(ray, plane);
	p = t*ray.direction + ray.origin;
	p.x -= 0.000001f;
	p.z -= 0.000001f;
	if (t && d.x*p.x < d.x*plane.point.x && d.z*p.z < d.z*plane.point.z)
		node = node->getAdjacentNode(2, p, d.y > 0.0f, depth);

	plane.normal = Vec3(0.0f, 0.0f, d.z);
	t = intersectsPlane(ray, plane);
	p = t*ray.direction + ray.origin;
	p.y -= 0.000001f;
	p.x -= 0.000001f;
	if (t && d.y*p.y < d.y*plane.point.y && d.x*p.x < d.x*plane.point.x)
		node = node->getAdjacentNode(4, p, d.z > 0.0f, depth);

	return node;
}

Node *Node::getAdjacentNode(int axis, Vec3 point, bool positive, int depth)
{
	if (!getParent())
		return this;
	else if (positive)
		return getNextNode(axis, point, depth);
	else
		return getPreviousNode(axis, point, depth);
}

Node *Node::getNextNode(int axis, Vec3 point, int depth)
{
	Node *node = this;
	Node *pnode = node->getParent();
	if (!pnode)
		return nullptr;
	/* A set bit is in the positive direction, and an unset bit is in the
	negative direction (x=1, y=2, z=4). */
	int index = node - pnode->getNode(0);
	if ((index & axis) == axis)
		node = pnode->getNextNode(axis, point, depth);
	else
		node = pnode->getNode(index + axis);

	if (node && node->getNode(0) && node->depth < depth)
		node = node->getChildNode(point, depth);
	return node ? node : this;
}

Node *Node::getPreviousNode(int axis, Vec3 point, int depth)
{
	Node *node = this;
	Node *pnode = node->getParent();
	if (!pnode)
		return nullptr;
	int index = node - pnode->getNode(0);
	if ((index & axis) == 0)
		node = pnode->getPreviousNode(axis, point, depth);
	else
		node = pnode->getNode(index - axis);
	if (node && node->getNode(0) && node->depth < depth)
		node = node->getChildNode(point, depth);
	return node ? node : this;
}

Node *Node::getChildNode(Vec3 point, int depth)
{
	Node *pnode = this;
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
	if (node->getNode(0) && node->depth < depth)
		return node->getChildNode(point, depth);
	else
		return node;
}

Node *Volume::getRoot()
{
	return &this->root;
}

const Node *Volume::getRoot() const
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
	depth(0),
	center(center),
	size(size),
	density(0.0f),
	direction(0.0f, 0.0f, 0.0f),
	quantity(0)
{

}

Node::Node() :
	nodes(nullptr),
	parent(nullptr),
	depth(0),
	density(0.0f),
	direction(0.0f, 0.0f, 0.0f),
	quantity(0)
{

}

Node::~Node()
{
	if (this->nodes)
		delete[] this->nodes;
}

void Node::clear()
{
	this->density = 0.0f;
	this->direction = Vec3(0.0f, 0.0f, 0.0f);
	this->quantity = 0;
	if (this->nodes) {
		delete[] this->nodes;
		this->nodes = nullptr;
	}
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

const Node *Node::getNode(int index) const
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

void Node::setDensity(float density)
{
	this->density = density;
}

float Node::getDensity() const
{
	return this->density;
}

void Node::setDirection(Vec3 direction)
{
	this->direction = direction;
}

Vec3 Node::getDirection() const
{
	return this->direction;
}

void Node::setQuantity(int quantity)
{
	this->quantity = quantity;
}

int Node::getQuantity() const
{
	return this->quantity;
}

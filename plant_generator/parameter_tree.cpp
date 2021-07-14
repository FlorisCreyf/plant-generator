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

#include "parameter_tree.h"
#include <assert.h>

using std::string;
using std::vector;
using namespace pg;

const float pi = 3.14159265359f;


GeneratorState::GeneratorState() : concentration(0.0f), node(0)
{

}

StemData::StemData() :
	densityCurve(1),
	inclineCurve(2),
	density(0.0f),
	distance(0.0f),
	length(60.0f),
	angleVariation(0.5f),
	radiusThreshold(0.01f),
	inclineVariation(0.0f),
	radiusVariation(0.1f),
	gravity(0.1f),
	radius(1.0f),
	fork(0.0f),
	forkAngle(0.5f),
	noise(0.05f)
{

}

LeafData::LeafData() :
	densityCurve(1),
	scale(1.0f, 1.0f, 1.0f),
	density(0.0f),
	distance(2.0f),
	rotation(pi),
	minUp(0.0f),
	maxUp(1.0f),
	localUp(1.0f),
	globalUp(1.0f),
	minForward(0.0f),
	maxForward(1.0f),
	gravity(0.0f),
	leavesPerNode(1)
{

}

ParameterNode::ParameterNode() :
	child(nullptr),
	parent(nullptr),
	nextSibling(nullptr),
	prevSibling(nullptr),
	data()
{

}

StemData ParameterNode::getData() const
{
	return this->data;
}

void ParameterNode::setData(StemData data)
{
	this->data = data;
}

const ParameterNode *ParameterNode::getChild() const
{
	return this->child;
}

const ParameterNode *ParameterNode::getSibling() const
{
	return this->nextSibling;
}

const ParameterNode *ParameterNode::getNextSibling() const
{
	return this->nextSibling;
}

const ParameterNode *ParameterNode::getPrevSibling() const
{
	return this->prevSibling;
}

const ParameterNode *ParameterNode::getParent() const
{
	return this->parent;
}

ParameterTree::ParameterTree() : root(nullptr)
{

}

ParameterTree::ParameterTree(const ParameterTree &original) : root(nullptr)
{
	copy(original);
}

ParameterTree::~ParameterTree()
{
	if (this->root) {
		removeChildNode(this->root->child);
		delete this->root;
	}
}

void ParameterTree::removeChildNode(ParameterNode *node)
{
	if (node) {
		removeChildNode(node->nextSibling);
		removeChildNode(node->child);
		delete node;
	}
}

ParameterTree &ParameterTree::operator=(const ParameterTree &tree)
{
	reset();
	copy(tree);
	return *this;
}

void ParameterTree::copyNode(const ParameterNode *originalNode,
	ParameterNode *node)
{
	if (!originalNode)
		return;
	if (originalNode->child) {
		node->child = new ParameterNode();
		node->child->parent = node;
		node->child->data = originalNode->child->data;
		copyNode(originalNode->child, node->child);
	}
	if (originalNode->nextSibling) {
		node->nextSibling = new ParameterNode();
		node->nextSibling->parent = node->parent;
		node->nextSibling->prevSibling = node;
		node->nextSibling->data = originalNode->nextSibling->data;
		copyNode(originalNode->nextSibling, node->nextSibling);
	}
}

void ParameterTree::copy(const ParameterTree &tree)
{
	if (tree.root) {
		this->root = new ParameterNode();
		this->root->data = tree.root->data;
		if (tree.root->child) {
			this->root->child = new ParameterNode();
			this->root->child->data = tree.root->child->data;
			copyNode(tree.root->child, this->root->child);
		}
	} else
		this->root = nullptr;
}

void ParameterTree::reset()
{
	if (this->root) {
		removeChildNode(this->root->child);
		delete this->root;
		this->root = nullptr;
	}
}

ParameterNode *ParameterTree::getRoot() const
{
	return this->root;
}

ParameterNode *ParameterTree::createRoot()
{
	reset();
	this->root = new ParameterNode();
	return this->root;
}

ParameterNode *ParameterTree::getNode() const
{
	return this->root ? this->root->child : nullptr;
}

ParameterNode *ParameterTree::addChild(string name)
{
	if (!this->root)
		return nullptr;
	else if (name.empty()) {
		ParameterNode *child = this->root->child;
		this->root->child = new ParameterNode();
		this->root->child->data.densityCurve.setDefault(1);
		this->root->child->data.leaf.densityCurve.setDefault(1);
		if (child)
			this->root->child->prevSibling = child;
		return this->root->child;
	} else {
		ParameterNode *node = getNode(name, 0, this->root->child);
		if (!node)
			return nullptr;
		ParameterNode *child = node->child;
		node->child = new ParameterNode();
		node->child->data.densityCurve.setDefault(1);
		node->child->data.leaf.densityCurve.setDefault(1);
		node->child->parent = node;
		node->child->nextSibling = child;
		if (child)
			child->prevSibling = node->child;
		return node->child;
	}
}

ParameterNode *ParameterTree::addSibling(string name)
{
	if (!this->root)
		return nullptr;
	ParameterNode *node = getNode(name, 0, this->root->child);
	if (!node)
		return nullptr;
	ParameterNode *sibling = node->nextSibling;
	node->nextSibling = new ParameterNode();
	node->nextSibling->data.densityCurve.setDefault(1);
	node->nextSibling->data.leaf.densityCurve.setDefault(1);
	node->nextSibling->parent = node->parent;
	node->nextSibling->nextSibling = sibling;
	node->nextSibling->prevSibling = node;
	return node->nextSibling;
}

ParameterNode *ParameterTree::get(string name) const
{
	if (name.empty() || !this->root)
		return nullptr;
	else
		return getNode(name, 0, this->root->child);
}

bool ParameterTree::remove(string name)
{
	if (!this->root)
		return false;
	if (name.empty()) {
		reset();
		return true;
	}

	ParameterNode *node = getNode(name, 0, this->root->child);
	if (!node)
		return false;

	if (node == this->root->child)
		this->root->child = node->nextSibling;
	if (node->prevSibling)
		node->prevSibling->nextSibling = node->nextSibling;
	if (node->nextSibling)
		node->nextSibling->prevSibling = node->prevSibling;
	if (node->parent && node->parent->child == node)
		node->parent->child = node->nextSibling;

	removeChildNode(node->child);
	delete node;
	return true;
}

vector<string> ParameterTree::getNames() const
{
	vector<string> names;
	if (this->root)
		getNames(names, "", this->root->child);
	return names;
}

void ParameterTree::getNames(vector<string> &names, string prefix,
	ParameterNode *node) const
{
	int count = 0;
	while (node) {
		string name = prefix + std::to_string(++count);
		names.push_back(name);
		getNames(names, name + ".", node->child);
		node = node->nextSibling;
	}
}

ParameterNode *ParameterTree::getNode(const string &name, size_t start,
	ParameterNode *node) const
{
	int size = getSize(name, start);
	for (int i = 0; i < size-1 && node; i++)
		node = node->nextSibling;

	if (!node)
		return nullptr;
	else if (start < name.size())
		return getNode(name, start, node->child);
	else
		return node;
}

int ParameterTree::getSize(const string &name, size_t &start) const
{
	size_t end = name.find('.', start);
	if (end != string::npos) {
		int size = std::stoi(name.substr(start, end-start));
		start = end + 1;
		return size;
	} else {
		int size = std::stoi(name.substr(start, name.size()));
		start = name.size();
		return size;
	}
}

void ParameterTree::updateField(std::function<void(StemData *)> function,
	std::string name)
{
	ParameterNode *node = get(name);
	function(&node->data);
}

void ParameterTree::updateFields(std::function<void(StemData *)> function)
{
	updateFields(function, this->root->child);
}

void ParameterTree::updateFields(std::function<void(StemData *)> function,
	ParameterNode *node)
{
	function(&node->data);
	if (node->nextSibling)
		updateFields(function, node->nextSibling);
	if (node->child)
		updateFields(function, node->child);
}

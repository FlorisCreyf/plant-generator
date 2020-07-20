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

#include "derivation.h"
#include <assert.h>

using std::string;
using std::vector;
using namespace pg;

Derivation DerivationNode::getData() const
{
	return this->data;
}

void DerivationNode::setData(Derivation data)
{
	this->data = data;
}

const DerivationNode *DerivationNode::getChild() const
{
	return this->child;
}

const DerivationNode *DerivationNode::getSibling() const
{
	return this->nextSibling;
}

const DerivationNode *DerivationNode::getNextSibling() const
{
	return this->nextSibling;
}

const DerivationNode *DerivationNode::getPrevSibling() const
{
	return this->prevSibling;
}

const DerivationNode *DerivationNode::getParent() const
{
	return this->parent;
}

DerivationTree::DerivationTree() : root(nullptr)
{

}

DerivationTree::DerivationTree(const DerivationTree &original)
{
	copy(original);
}

DerivationTree::~DerivationTree()
{
	if (this->root)
		removeChildNode(this->root);
}

void DerivationTree::removeChildNode(DerivationNode *node)
{
	if (node != nullptr) {
		removeChildNode(node->nextSibling);
		removeChildNode(node->child);
		delete node;
	}
}

DerivationTree &DerivationTree::operator=(const DerivationTree &derivation)
{
	reset();
	copy(derivation);
	return *this;
}

void DerivationTree::copyNode(
	const DerivationNode *originalNode, DerivationNode *node)
{
	if (originalNode->child) {
		node->child = new DerivationNode();
		node->child->parent = node;
		node->child->data = originalNode->child->data;
		copyNode(originalNode->child, node->child);
	}
	if (originalNode->nextSibling) {
		node->nextSibling = new DerivationNode();
		node->nextSibling->parent = node->parent;
		node->nextSibling->prevSibling = node;
		node->nextSibling->data = originalNode->nextSibling->data;
		copyNode(originalNode->nextSibling, node->nextSibling);
	}
}

void DerivationTree::copy(const DerivationTree &derivation)
{
	this->seed = derivation.seed;
	if (derivation.root) {
		this->root = new DerivationNode();
		this->root->data = derivation.root->data;
		copyNode(derivation.root, this->root);
		copyNode(derivation.root, this->root);
	} else
		this->root = nullptr;
}

void DerivationTree::reset()
{
	removeChildNode(this->root);
	this->root = nullptr;
}

void DerivationTree::setSeed(int seed)
{
	this->seed = seed;
}

int DerivationTree::getSeed() const
{
	return this->seed;
}

DerivationNode *DerivationTree::getRoot() const
{
	return this->root;
}

DerivationNode *DerivationTree::createRoot()
{
	if (this->root)
		removeChildNode(this->root);
	this->root = new DerivationNode;
	this->root->data.stemDensityCurve.setDefault(1);
	this->root->data.leafDensityCurve.setDefault(1);
	return this->root;
}

DerivationNode *DerivationTree::addChild(string name)
{
	DerivationNode *node = getNode(name, 0, this->root);
	if (!node)
		return nullptr;
	DerivationNode *child = node->child;
	node->child = new DerivationNode;
	node->child->data.stemDensityCurve.setDefault(1);
	node->child->data.leafDensityCurve.setDefault(1);
	node->child->parent = node;
	node->child->nextSibling = child;
	if (child)
		child->prevSibling = node->child;
	return node->child;
}

DerivationNode *DerivationTree::addSibling(string name)
{
	DerivationNode *node = getNode(name, 0, this->root);
	if (!node)
		return nullptr;
	DerivationNode *sibling = node->nextSibling;
	node->nextSibling = new DerivationNode;
	node->nextSibling->data.stemDensityCurve.setDefault(1);
	node->nextSibling->data.leafDensityCurve.setDefault(1);
	node->nextSibling->parent = node->parent;
	node->nextSibling->nextSibling = sibling;
	node->nextSibling->prevSibling = node;
	return node->nextSibling;
}

DerivationNode *DerivationTree::get(string name) const
{
	if (name.empty())
		return this->root;
	else
		return getNode(name, 0, this->root);
}

bool DerivationTree::remove(string name)
{
	DerivationNode *node = getNode(name, 0, this->root);
	if (!node)
		return false;

	if (node == this->root) {
		assert(node->prevSibling == nullptr);
		this->root = node->nextSibling;
	}
	if (node->prevSibling)
		node->prevSibling->nextSibling = node->nextSibling;
	if (node->nextSibling)
		node->nextSibling->prevSibling = node->prevSibling;
	if (node->parent && node->parent->child == node) {
		assert(node->prevSibling == nullptr);
		node->parent->child = node->nextSibling;
	}
	removeChildNode(node->child);
	delete node;
	return true;
}

vector<string> DerivationTree::getNames() const
{
	vector<string> names;
	getNames(names, "", this->root);
	return names;
}

void DerivationTree::getNames(
	vector<std::string> &names, string prefix, DerivationNode *node) const
{
	int count = 0;
	while (node) {
		string name = prefix + std::to_string(++count);
		names.push_back(name);
		getNames(names, name + ".", node->child);
		node = node->nextSibling;
	}
}

DerivationNode *DerivationTree::getNode(
	const string &name, size_t start, DerivationNode *node) const
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

int DerivationTree::getSize(const string &name, size_t &start) const
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

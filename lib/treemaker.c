/* TreeMaker: 3D tree model generator
 * Copyright (C) 2016-2017  Floris Creyf
 *
 * TreeMaker is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TreeMaker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "treemaker.h"
#include "tree_struct.h"
#include "model_builder.h"
#include "struct_modifier.h"
#include "tree_data.h"
#include "array.h"
#include "node.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

struct __TMtree {
	Node *root;
	TreeData td;
	float *vertexBuffer;
	unsigned short *elementBuffer;
};

Node *findNode(Node *parentNode, int *i, int name)
{
	Node *childNode;
	int j;

	if ((*i)++ == name)
		return parentNode;

	for (j = 0; j < parentNode->branchCount; j++) {
		childNode = findNode(&(parentNode->branches[j]), i, name);
		if (childNode != NULL)
			return childNode;
	}

	return NULL;
}

Node *getNode(Node *n, int name)
{
	int i = 0;
	return findNode(n, &i, name);
}

int tmGetBranchPathSize(TMtree tree, int name)
{
	Node *n = getNode(tree->root, name);
	return n ? n->lineCount + 1 : 0;
}

void tmGetBranchPath(TMtree tree, int name, TMvec3 *data)
{
	Node *n = getNode(tree->root, name);

	if (n) {
		int i;
		for (i = 0; i < n->lineCount; i++) {
			data[i] = n->lines[i].start;
			data[i] = tmAddVec3(&data[i], &n->globPos);
		}
		data[i] = getLineEndPoint(&n->lines[i-1]);
		data[i] = tmAddVec3(&data[i], &n->globPos);
	}
}

void tmSetBranchPoint(TMtree tree, int name, TMvec3 point, int index)
{
	Node *n = getNode(tree->root, name);
	Line *line;

	if (!n)
		return;

	point = tmSubVec3(&point, &n->globPos);

	if (index > 0) {
		TMvec3 s;
		line = &n->lines[index-1];
		s = tmSubVec3(&point, &line->start);
		line->length = tmMagVec3(&s);
		tmNormalizeVec3(&s);
		line->direction = s;
	}

	if (index < n->lineCount) {
		TMvec3 a = getLineEndPoint(&n->lines[index]);
		TMvec3 b = tmSubVec3(&a, &point);
		n->lines[index].length = tmMagVec3(&b);
		tmNormalizeVec3(&b);
		n->lines[index].direction = b;
		n->lines[index].start = point;
	}
}

void tmSetRadius(TMtree tree, int name, float radius)
{
	Node *n = getNode(tree->root, name);
	if (n)
		n->radius = radius;
}

void tmSetRadiusCurve(TMtree tree, int name, TMvec3 *curve, int size)
{
	Node *n = getNode(tree->root, name);
	int ts = sizeof(TMvec3);
	void *p;
	p = fillArray(n->radiusCurve, n->radiusCurveSize, curve, size, ts);
	n->radiusCurve = p;
	n->radiusCurveSize = size;
}

void tmSetBranchCurve(TMtree tree, int name, TMvec3 *curve, int size)
{
	Node *n = getNode(tree->root, name);
	int ts = sizeof(TMvec3);
	void *p;
	p = fillArray(n->branchCurve, n->branchCurveSize, curve, size, ts);
	n->branchCurve = p;
	n->branchCurveSize = size;
}

void tmSetResolution(TMtree tree, int name, int resolution)
{
	Node *n = getNode(tree->root, name);
	if (n) {
		n->resolution = resolution;
		modifyResolutions(n, resolution);
	}
}

void tmSetCrownBaseHeight(TMtree tree, float cbh)
{
	tree->td.crownBaseHeight = cbh;
}

float tmGetCrownBaseHeight(TMtree tree)
{
	return tree->td.crownBaseHeight;
}

void tmSetBranchDensity(TMtree tree, int name, float density)
{
	Node *n = getNode(tree->root, name);
	if (n && n->branchDensity != density) {
		Position p;
		int dichotomous = n->dichotomousStart;

		if (tree->root == n)
			p = getPathStartPosition(n);
		else {
			p.t = 0.0f;
			p.index = 0;
		}
		n->branchDensity = density;

		removeNodes(n);
		addLateralBranches(n, p);
		if (dichotomous >= 0)
			addDichotomousBranches(n);
	}
}

float tmGetBranchDensity(TMtree tree, int name)
{
	Node *n = getNode(tree->root, name);
	return n ? n->branchDensity : 0.0f;
}

void tmSetCrossSections(TMtree tree, int name, int sections)
{
	Node *n = getNode(tree->root, name);
	if (n)
		n->crossSections = sections;
}

void tmSetMaxBranchDepth(TMtree tree, int depth)
{
	tree->td.maxBranchDepth = depth;
}

int tmGetCrossSections(TMtree tree, int name)
{
	Node *n = getNode(tree->root, name);
	return n ? n->crossSections : -1;
}

int tmGetResolution(TMtree tree, int name)
{
	Node *n = getNode(tree->root, name);
	return n ? n->resolution : -1;
}

float tmGetRadius(TMtree tree, int name)
{
	Node *n = getNode(tree->root, name);
	return n ? n->radius : -1.0f;
}

void tmGetRadiusCurve(TMtree tree, int name, TMvec3 **curve, int *size)
{
	Node *n = getNode(tree->root, name);
	if (n) {
		*curve = n->radiusCurve;
		*size = n->radiusCurveSize;
	}
}

void tmGetBranchCurve(TMtree tree, int name, TMvec3 **curve, int *size)
{
	Node *n = getNode(tree->root, name);
	if (n) {
		*curve = n->branchCurve;
		*size = n->branchCurveSize;
	}
}

TMaabb tmGetBoundingBox(TMtree tree, int name)
{
	Node *n = getNode(tree->root, name);
	return n ? n->bounds : (TMaabb){0, 0, 0, 0, 0, 0};
}

int tmGetIBOStart(TMtree tree, int name)
{
	Node *n = getNode(tree->root, name);
	return n->iboStart;
}

int tmGetIBOEnd(TMtree tree, int name)
{
	Node *n = getNode(tree->root, name);
	return n->iboEnd;
}

int tmIsTerminalBranch(TMtree tree, int name)
{
	Node *n = getNode(tree->root, name);
	return n->terminal;
}

int tmGetVBOSize(TMtree tree)
{
	return tree->td.vboSize;
}

int tmGetIBOSize(TMtree tree)
{
	return tree->td.iboSize;
}

void tmGenerateStructure(TMtree tree)
{
	resetTreeStructure(tree->root);
	newTreeStructure(&tree->td, tree->root);
}

int tmGenerateMesh(TMtree tree, float *v, int vs, unsigned short *i, int is)
{
	int status = buildModel(v, vs, i, is, tree->root);
	tree->td.iboSize = getIBOSize();
	tree->td.vboSize = getVBOSize();
	return status;
}

TMtree tmNewTree()
{
	srand(time(NULL));
	TMtree tree = (TMtree)malloc(sizeof(struct __TMtree));
	tree->root = newNodes(1);
	tree->td.vboSize = 0;
	tree->td.iboSize = 0;
	return tree;
}

void tmDeleteTree(TMtree tree)
{
	if (tree != NULL) {
		freeTreeStructure(tree->root);
		free(tree);
	}
}

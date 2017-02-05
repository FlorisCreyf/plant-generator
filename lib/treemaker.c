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
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

struct __TMtree {
	Node *root;
	TreeData td;
	float *vertexBuffer;
	unsigned short *elementBuffer;
};

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

Node *findNode(Node *n, int *i, int id)
{
	Node *m;
	int j;

	if ((*i)++ == id)
		return n;

	for (j = 0; j < n->branchCount; j++) {
		m = findNode(&(n->branches[j]), i, id);
		if (m != NULL)
			return m;
	}

	return NULL;
}

Node *getNode(Node *n, int id)
{
	int i = 0;
	return findNode(n, &i, id);
}

void tmSetRadius(TMtree tree, int id, float radius)
{
	Node *n = getNode(tree->root, id);
	if (n)
		n->radius = radius;
}

void tmSetRadiusCurve(TMtree tree, int id, TMvec3 *curve, int size)
{
	Node *n = getNode(tree->root, id);
	int ts = sizeof(TMvec3);
	void *p;
	p = fillArray(n->radiusCurve, n->radiusCurveSize, curve, size, ts);
	n->radiusCurve = p;
	n->radiusCurveSize = size;
}

void tmSetBranchCurve(TMtree tree, int id, TMvec3 *curve, int size)
{
	Node *n = getNode(tree->root, id);
	int ts = sizeof(TMvec3);
	void *p;
	p = fillArray(n->branchCurve, n->branchCurveSize, curve, size, ts);
	n->branchCurve = p;
	n->branchCurveSize = size;
}

void tmSetResolution(TMtree tree, int id, int resolution)
{
	Node *n = getNode(tree->root, id);
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

void tmSetBranchDensity(TMtree tree, int id, float density)
{
	Node *n = getNode(tree->root, id);
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

float tmGetBranchDensity(TMtree tree, int id)
{
	Node *n = getNode(tree->root, id);
	return n ? n->branchDensity : 0.0f;
}

void tmSetCrossSections(TMtree tree, int id, int sections)
{
	Node *n = getNode(tree->root, id);
	if (n)
		n->crossSections = sections;
}

void tmSetMaxBranchDepth(TMtree tree, int depth)
{
	tree->td.maxBranchDepth = depth;
}

int tmGetCrossSections(TMtree tree, int id)
{
	Node *n = getNode(tree->root, id);
	return n ? n->crossSections : -1;
}

int tmGetResolution(TMtree tree, int id)
{
	Node *n = getNode(tree->root, id);
	return n ? n->resolution : -1;
}

float tmGetRadius(TMtree tree, int id)
{
	Node *n = getNode(tree->root, id);
	return n ? n->radius : -1.0f;
}

void tmGetRadiusCurve(TMtree tree, int id, TMvec3 **curve, int *size)
{
	Node *n = getNode(tree->root, id);
	if (n) {
		*curve = n->radiusCurve;
		*size = n->radiusCurveSize;
	}
}

void tmGetBranchCurve(TMtree tree, int id, TMvec3 **curve, int *size)
{
	Node *n = getNode(tree->root, id);
	if (n) {
		*curve = n->branchCurve;
		*size = n->branchCurveSize;
	}
}

TMaabb tmGetBoundingBox(TMtree tree, int id)
{
	Node *n = getNode(tree->root, id);
	return n ? n->bounds : (TMaabb){0, 0, 0, 0, 0, 0};
}

int tmGetIBOStart(TMtree tree, int id)
{
	Node *n = getNode(tree->root, id);
	return n->iboStart;
}

int tmGetIBOEnd(TMtree tree, int id)
{
	Node *n = getNode(tree->root, id);
	return n->iboEnd;
}

int tmIsTerminalBranch(TMtree tree, int id)
{
	Node *n = getNode(tree->root, id);
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

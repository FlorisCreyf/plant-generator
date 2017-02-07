/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef TREEMAKER_H
#define TREEMAKER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "collision.h"

typedef struct __TMtree *TMtree;

TMtree tmNewTree();
void tmDeleteTree(TMtree tree);

int tmGetBranchLineSize(TMtree tree, int branch);
void tmGetBranchLine(TMtree, int branch, TMvec3 *data);

void tmSetRadius(TMtree tree, int id, float radius);
float tmGetRadius(TMtree tree, int id);

void tmSetRadiusCurve(TMtree tree, int id, TMvec3 *curve, int size);
void tmGetRadiusCurve(TMtree tree, int id, TMvec3 **curve, int *size);

void tmSetBranchCurve(TMtree tree, int id, TMvec3 *curve, int size);
void tmGetBranchCurve(TMtree tree, int id, TMvec3 **curve, int *size);

void tmSetCrossSections(TMtree tree, int id, int sections);
int tmGetCrossSections(TMtree tree, int id);

void tmSetResolution(TMtree tree, int id, int resolution);
int tmGetResolution(TMtree tree, int id);

void tmSetCrownBaseHeight(TMtree tree, float cbh);
float tmGetCrownBaseHeight(TMtree tree);

void tmSetBranchDensity(TMtree tree, int id, float density);
float tmGetBranchDensity(TMtree, int id);

void tmSetMaxBranchDepth(TMtree tree, int depth);
TMaabb tmGetBoundingBox(TMtree tree, int id);
int tmIsTerminalBranch(TMtree tree, int id);

int tmGetIBOStart(TMtree tree, int id);
int tmGetIBOEnd(TMtree tree, int id);
int tmGetVBOSize(TMtree tree);
int tmGetIBOSize(TMtree tree);

void tmGenerateStructure(TMtree tree);
int tmGenerateMesh(TMtree tree, float *v, int vs, unsigned short *i, int is);

#ifdef __cplusplus
}
#endif

#endif /* TREEMAKER_H  */

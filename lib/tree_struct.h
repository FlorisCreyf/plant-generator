/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef TREE_STRUCT_H
#define TREE_STRUCT_H

#include "node.h"
#include "tree_data.h"

Node *newTreeStructure(TreeData *data, Node *root);
void freeTreeStructure(Node *root);
void resetTreeStructure(Node *root);
struct Position getPathStartPosition(Node *n);
void addLateralBranches(Node *stem, struct Position pos);
TMvec3 addDichotomousBranches(Node *stem);

#endif /* TREE_STRUCT_H */

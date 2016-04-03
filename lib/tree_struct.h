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

node *new_tree_structure(tree_data *td);
void free_tree_structure(node *root);

#endif /* TREE_STRUCT_H */


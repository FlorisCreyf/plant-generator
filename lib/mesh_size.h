/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef MESH_SIZE_H
#define MESH_SIZE_H

#include "node.h"

int get_branch_vcount(node *stem)
{
	return stem->resolution * 3 * 2;
}

int get_branch_ecount(node *stem)
{
	return stem->resolution * 6;
}

int get_cap_ecount(node *stem)
{
	return stem->resolution/2 * 5;
}

int get_dichotomous_ecount(node *stem)
{
	return (int)((stem->resolution+1.f + (stem->resolution/2.f)) * 6.f);
}

#endif /* MESH_SIZE_H */

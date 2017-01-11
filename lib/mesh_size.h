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

int getBranchVCount(Node *stem)
{
	return stem->resolution * 3 * 2;
}

int getBranchICount(Node *stem)
{
	return stem->resolution * 6;
}

int getCapICount(Node *stem)
{
	int odd = stem->resolution & 1 == 0 ? 0 : 3;
	int squares = stem->resolution == 3 ? 3 : stem->resolution/2 - 1 * 6;
	return squares + odd;
}

int getDichotomousICount(Node *stem)
{
	return (int)((stem->resolution+1.f + (stem->resolution/2.f)) * 6.f);
}

#endif /* MESH_SIZE_H */

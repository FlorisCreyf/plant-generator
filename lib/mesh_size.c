/* 
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "mesh_size.h"

int get_branch_vcount(node *stem)
{
	return stem->cross_sections * stem->branch_resolution * 3;
}

int get_dichotomous_seam_size()
{
	return 0;
}


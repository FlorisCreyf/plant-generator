/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

 #include "struct_modifier.h"

 void modify_resolutions(node *stem, int r)
 {
 	int i;
 	if (stem->branch_count == 0 && stem->dichotomous_start < 0)
 		return;
 	for (i = stem->dichotomous_start; i < stem->branch_count; i++) {
 		stem->branches[i].resolution = r;
 		modify_resolutions(&stem->branches[i], r);
 	}
 }

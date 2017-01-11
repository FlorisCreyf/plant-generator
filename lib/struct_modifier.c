/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

 #include "struct_modifier.h"

 void modifyResolutions(Node *stem, int r)
 {
 	int i;
 	if (stem->branchCount == 0 && stem->dichotomousStart < 0)
 		return;
 	for (i = stem->dichotomousStart; i < stem->branchCount; i++) {
 		stem->branches[i].resolution = r;
 		modifyResolutions(&stem->branches[i], r);
 	}
 }

/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "array.h"
#include <stdlib.h>
#include <string.h>

void *fillArray(void *dest, int dSize, void *orig, int oSize, int typeSize)
{
	if (dSize > 0)
		dest = realloc(dest, oSize * typeSize);
	else
		dest = malloc(oSize * typeSize);

	memcpy(dest, orig, oSize * typeSize);
	return dest;
}

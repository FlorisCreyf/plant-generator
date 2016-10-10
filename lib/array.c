/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "array.h"

void set_array(void **dest, int *dsize, void *orig, int osize, int type_size)
{
	if (*dsize > 0)
		*dest = realloc(*dest, osize * type_size);
	else
		*dest = malloc(osize * type_size);

	memcpy(*dest, orig, osize * type_size);
	*dsize = osize;
}

void expand(void **arr, int *capacity, int size)
{
	*capacity *= 2;
	size *= *capacity;
	*arr = realloc(*arr, size);
}

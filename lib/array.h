/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef ARRAY_H
#define ARRAY_H

void set_array(void **dest, int *dsize, void *orig, int osize, int type_size);
void expand(void **arr, int *capacity, int size);

#endif /* ARRAY_H */

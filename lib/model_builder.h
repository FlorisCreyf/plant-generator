/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef MODEL_BUILDER_H
#define MODEL_BUILDER_H

#include "node.h"

int get_vbo_size();
int get_ebo_size();
int build_model(float *vb, int vb_size, unsigned short *eb, int eb_size, 
		node *root);

#endif /* MODEL_BUILDER_H */

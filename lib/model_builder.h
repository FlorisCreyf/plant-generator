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

void build_model(float *vertex_buffer, unsigned short *element_buffer, 
		node *root);

#endif /* MODEL_BUILDER_H */


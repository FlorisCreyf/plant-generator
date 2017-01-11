/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef COLLISION_H
#define COLLISION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vector.h"
#include <stdio.h>

typedef struct TMbo {
        float x1, x2;
        float y1, y2;
        float z1, z2;
} TMaabb;

typedef struct TMobb {
        float h[3];
        TMvec3 n[3];
        TMvec3 center;
} TMobb;

TMaabb tmCreateAABB(float *buffer, int size);
float tmIntersectsOBB(TMvec3 origin, TMvec3 direction, TMobb obb);
float tmIntersectsAABB(TMvec3 origin, TMvec3 direction, TMaabb aabb);

#ifdef __cplusplus
}
#endif

#endif /* COLLISION_H */

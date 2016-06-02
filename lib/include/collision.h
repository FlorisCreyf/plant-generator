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

typedef struct bt_bo_tag {
        float x1, x2;
        float y1, y2;
        float z1, z2;
} bt_aabb;

typedef struct bt_obb_tag {
        float h[3];
        bt_vec3 n[3];
        bt_vec3 center;
} bt_obb;

bt_aabb bt_create_aabb(float *buffer, int size);
float bt_intersects_obb(bt_vec3 origin, bt_vec3 direction, bt_obb obb);
float bt_intersects_aabb(bt_vec3 origin, bt_vec3 direction, bt_aabb aabb);

#ifdef __cplusplus
}
#endif

#endif /* COLLISION_H */

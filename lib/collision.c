/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "collision.h"
#include <math.h>
#include <float.h>

tm_aabb tm_create_aabb(float *buffer, int size)
{
        tm_aabb aabb = {
                buffer[0], buffer[0],
                buffer[1], buffer[1],
                buffer[2], buffer[2]};
        int i;

        for (i = 0; i < size; i += 6) {
                if (aabb.x1 > buffer[i])
                        aabb.x1 = buffer[i];
                if (aabb.x2 < buffer[i])
                        aabb.x2 = buffer[i];
                if (aabb.y1 > buffer[i+1])
                        aabb.y1 = buffer[i+1];
                if (aabb.y2 < buffer[i+1])
                        aabb.y2 = buffer[i+1];
                if (aabb.z1 > buffer[i+2])
                        aabb.z1 = buffer[i+2];
                if (aabb.z2 < buffer[i+2])
                        aabb.z2 = buffer[i+2];
        }

        return aabb;
}

void swap(float *a, float *b)
{
        float t = *b;
        *b = *a;
        *a = t;
}

float tm_intersects_obb(tm_vec3 origin, tm_vec3 direction, tm_obb obb)
{
        float e, f, t1, t2;
        float tmin = FLT_MIN;
        float tmax = FLT_MAX;
        tm_vec3 p = tm_sub_vec3(&obb.center, &origin);
        int i;

        for (i = 0; i < 3; i++) {
                e = tm_dot_vec3(&obb.n[i], &p);
                f = tm_dot_vec3(&obb.n[i], &direction);

                if (abs(f) > 0.0001f) {
                        t1 = (e + obb.h[i]) / f;
                        t2 = (e - obb.h[i]) / f;

                        if (t1 > t2)
                                swap(&t1, &t2);
                        if (t1 > tmin)
                                tmin = t1;
                        if (t2 < tmax)
                                tmax = t2;
                        if (tmin > tmax)
                                return 0.0f;
                        if (tmax < 0.0f)
                                return 0.0f;
                } else if (-e - obb.h[i] > 0.0f || -e + obb.h[i] < 0.0f)
                        return 0.0f;
        }

        return tmin > 0 ? tmin : tmax;
}

float tm_intersects_aabb(tm_vec3 origin, tm_vec3 direction, tm_aabb aabb)
{
        tm_obb obb;
        obb.h[0] = (aabb.x2 - aabb.x1) * 0.5f;
        obb.h[1] = (aabb.y2 - aabb.y1) * 0.5f;
        obb.h[2] = (aabb.z2 - aabb.z1) * 0.5f;

        obb.center = (tm_vec3){
                (aabb.x1 + aabb.x2) * 0.5f,
                (aabb.y1 + aabb.y2) * 0.5f,
                (aabb.z1 + aabb.z2) * 0.5f,
        };

        obb.n[0] = (tm_vec3){1.0f, 0.0f, 0.0f};
        obb.n[1] = (tm_vec3){0.0f, 1.0f, 0.0f};
        obb.n[2] = (tm_vec3){0.0f, 0.0f, 1.0f};

        return tm_intersects_obb(origin, direction, obb);
}

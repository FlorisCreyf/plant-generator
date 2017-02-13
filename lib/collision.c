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
#include <stdlib.h>
#include <float.h>

TMaabb tmCreateAABB(float *buffer, int size)
{
        TMaabb aabb = {
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

float tmIntersectsOBB(TMvec3 origin, TMvec3 direction, TMobb obb)
{
        float e, f, t1, t2;
        float tmin = FLT_MIN;
        float tmax = FLT_MAX;
        TMvec3 p = tmSubVec3(&obb.center, &origin);
        int i;

        for (i = 0; i < 3; i++) {
                e = tmDotVec3(&obb.n[i], &p);
                f = tmDotVec3(&obb.n[i], &direction);

                if (absf(f) > 0.0001f) {
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

float tmIntersectsAABB(TMvec3 origin, TMvec3 direction, TMaabb aabb)
{
        TMobb obb;
        obb.h[0] = (aabb.x2 - aabb.x1) * 0.5f;
        obb.h[1] = (aabb.y2 - aabb.y1) * 0.5f;
        obb.h[2] = (aabb.z2 - aabb.z1) * 0.5f;

        obb.center = (TMvec3){
                (aabb.x1 + aabb.x2) * 0.5f,
                (aabb.y1 + aabb.y2) * 0.5f,
                (aabb.z1 + aabb.z2) * 0.5f,
        };

        obb.n[0] = (TMvec3){1.0f, 0.0f, 0.0f};
        obb.n[1] = (TMvec3){0.0f, 1.0f, 0.0f};
        obb.n[2] = (TMvec3){0.0f, 0.0f, 1.0f};

        return tmIntersectsOBB(origin, direction, obb);
}

float tmIntersectsPlane(TMray ray, TMplane plane)
{
        float a = tmDotVec3(&plane.normal, &ray.direction);
        if (a > 0.0f) {
                TMvec3 b = tmSubVec3(&plane.point, &ray.origin);
                return tmDotVec3(&b, &plane.normal) / a;
        }
        return 0.0f;
}

float tmIntersectsSphere(TMray ray, TMvec3 position, float radius)
{
        TMvec3 l = tmSubVec3(&position, &ray.origin);
        float a = tmDotVec3(&l, &ray.direction);
        float b = tmDotVec3(&l, &l);
        float c;
        float d;

        if (a < 0.0f && b > pow(radius, 2))
                return 0.0f;
        c = b - a*a;
        if (c > pow(radius, 2))
                return 0.0f;
        d = sqrt(pow(radius, 2) - c);

        return b > pow(radius, 2) ? a - d : a + d;
}

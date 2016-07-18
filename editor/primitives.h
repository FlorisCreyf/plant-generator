/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include "objects.h"
#include "collision.h"
#include <vector>

void createGrid(GeometryComponent &g, int sections, float scale);
void createBox(GeometryComponent &g, bt_aabb &b);
void createLine(GeometryComponent &g, std::vector<vec3> p);
void createBezier(GeometryComponent &g, std::vector<vec3> p, int resolution);
void createPlane(GeometryComponent &g, bt_vec3 a, bt_vec3 b);

#endif /* PRIMITIVES_H */

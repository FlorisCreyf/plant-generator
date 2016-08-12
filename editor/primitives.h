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

void createGrid(GeometryComponent &g, int sections, bt_vec3 color,
                bt_vec3 sectionColor, bt_mat4 transform = bt_mat4_identity());
void createBox(GeometryComponent &g, bt_aabb &b, bt_vec3 c);
void createLine(GeometryComponent &g, std::vector<vec3> p, bt_vec3 color);
void createBezier(GeometryComponent &g, std::vector<vec3> p, int resolution,
                bt_vec3 color);
void createPath(GeometryComponent &g, std::vector<bt_vec3> p, int resolution,
                bt_vec3 color);
void createPlane(GeometryComponent &g, bt_vec3 a, bt_vec3 b, bt_vec3 c);

#endif /* PRIMITIVES_H */

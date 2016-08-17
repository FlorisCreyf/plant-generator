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

void createGrid(GeometryComponent &g, int sections, tm_vec3 color,
                tm_vec3 sectionColor, tm_mat4 transform = tm_mat4_identity());
void createBox(GeometryComponent &g, tm_aabb &b, tm_vec3 c);
void createLine(GeometryComponent &g, std::vector<vec3> p, tm_vec3 color);
void createBezier(GeometryComponent &g, std::vector<vec3> p, int resolution,
                tm_vec3 color);
void createPath(GeometryComponent &g, std::vector<tm_vec3> p, int resolution,
                tm_vec3 color);
void createPlane(GeometryComponent &g, tm_vec3 a, tm_vec3 b, tm_vec3 c);

#endif /* PRIMITIVES_H */

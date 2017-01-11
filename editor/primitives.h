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

void createGrid(GeometryComponent &g, int sections, TMvec3 color,
                TMvec3 sectionColor, TMmat4 transform = tmMat4Identity());
void createBox(GeometryComponent &g, TMaabb &b, TMvec3 c);
void createLine(GeometryComponent &g, vector<TMvec3> p, TMvec3 color);
void createBezier(GeometryComponent &g, vector<TMvec3> p, int resolution,
                TMvec3 color);
void createPath(GeometryComponent &g, vector<TMvec3> p, int resolution,
                TMvec3 color);
void createPlane(GeometryComponent &g, TMvec3 a, TMvec3 b, TMvec3 c);

#endif /* PRIMITIVES_H */

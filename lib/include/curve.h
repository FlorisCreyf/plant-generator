/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef CURVE_H
#define CURVE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vector.h"

vec3 bt_get_bezier(float t, vec3 *p, int degree);

#ifdef __cplusplus
}
#endif

#endif /* CURVE_H */

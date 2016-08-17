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

tm_vec3 tm_get_bezier(float t, tm_vec3 *p, int degree);
tm_vec3 tm_get_path(float t, tm_vec3 *p, int curve_count);

#ifdef __cplusplus
}
#endif

#endif /* CURVE_H */

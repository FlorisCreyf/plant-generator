/* 
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef VECTOR_H
#define VECTOR_H

typedef struct bt_vec3_tag {
	union {
		float x;
		float r;
	};
	union {
		float y;
		float g;
	};
	union {
		float z;
		float b;
	};
} bt_vec3;

typedef struct bt_mat4_tag {
	float x1, x2, x3, x4;
	float y1, y2, y3, y4;
	float z1, z2, z3, z4;
	float w1, w2, w3, w4;
} bt_mat4;

int bt_dotvec3(bt_vec3 *a, bt_vec3 *b);
bt_vec3 bt_crossvec3(bt_vec3 *a, bt_vec3 *b);
bt_mat4 bt_rotate_into_vec(bt_vec3 *normal, bt_vec3 *direction);
void bt_add_translation(bt_mat4 *m, bt_vec3 *translation);
void bt_multiply_transform(bt_vec3 *vec, bt_mat4 *transform);

#endif /* VECTOR_H */


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

#ifdef __cplusplus
extern "C" {
#endif

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

typedef struct mat4_tag {
	float m[4][4];
} bt_mat4;

int bt_dot_vec3(bt_vec3 *a, bt_vec3 *b);
bt_vec3 bt_cross_vec3(bt_vec3 *a, bt_vec3 *b);
bt_vec3 bt_add_vec3(bt_vec3 *a, bt_vec3 *b);
bt_vec3 bt_sub_vec3(bt_vec3 *a, bt_vec3 *b);
void bt_normalize_vec3(bt_vec3 *a);
bt_mat4 bt_mult_mat4(bt_mat4 *a, bt_mat4 *b);
bt_mat4 bt_rotate_into_vec(bt_vec3 *normal, bt_vec3 *direction);
void bt_add_translation(bt_mat4 *m, bt_vec3 *translation);
void bt_point_transform(bt_vec3 *v, bt_mat4 *t);

#ifdef __cplusplus
}
#endif

#endif /* VECTOR_H */


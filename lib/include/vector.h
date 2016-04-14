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

typedef struct bt_vec4_tag {
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
	union {
		float w;
		float a;
	};
} bt_vec4;

typedef bt_vec4 bt_quat;

typedef struct bt_mat4_tag {
	float m[4][4];
} bt_mat4;

float bt_dot_vec3(bt_vec3 *a, bt_vec3 *b);
bt_vec3 bt_cross_vec3(bt_vec3 *a, bt_vec3 *b);
bt_vec3 bt_add_vec3(bt_vec3 *a, bt_vec3 *b);
bt_vec3 bt_sub_vec3(bt_vec3 *a, bt_vec3 *b);
void bt_normalize_vec3(bt_vec3 *a);
void bt_normalize_vec4(bt_vec4 *a);
bt_mat4 bt_mult_mat4(bt_mat4 *a, bt_mat4 *b);
bt_mat4 bt_rotate_into_vec(bt_vec3 *normal, bt_vec3 *direction);
bt_mat4 bt_translate(float x, float y, float z);
void bt_point_transform(bt_vec3 *v, bt_mat4 *t);
bt_quat bt_from_axis_angle(float x, float y, float z, float theta);
bt_mat4 bt_quat_to_mat4(bt_quat q);

#ifdef __cplusplus
}
#endif

#endif /* VECTOR_H */


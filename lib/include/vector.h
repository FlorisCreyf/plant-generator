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

typedef struct tm_vec3_tag {
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
} tm_vec3;

typedef struct tm_quat_tag {
	union {
		struct {
			float x;
			float y;
			float z;
		};
		tm_vec3 v;
	};
	float w;
} tm_quat;

typedef struct tm_mat4_tag {
	float m[4][4];
} tm_mat4;

typedef tm_mat4 mat4;
typedef tm_quat quat;
typedef tm_vec3 vec3;

float absf(float f);
float tm_dot_vec3(tm_vec3 *a, tm_vec3 *b);
tm_vec3 tm_cross_vec3(tm_vec3 *a, tm_vec3 *b);
tm_vec3 tm_add_vec3(tm_vec3 *a, tm_vec3 *b);
tm_vec3 tm_sub_vec3(tm_vec3 *a, tm_vec3 *b);
tm_vec3 tm_mult_vec3(float a, tm_vec3 *b);
float tm_mag_vec3(tm_vec3 *a);
void tm_normalize_vec3(tm_vec3 *a);
tm_mat4 tm_transpose_mat4(tm_mat4 *m);
tm_mat4 tm_mult_mat4(tm_mat4 *a, tm_mat4 *b);
tm_mat4 tm_rotate_into_vec(tm_vec3 *normal, tm_vec3 *direction);
tm_vec3 tm_rotate_around_axis(vec3 *v, vec3 *axis, float n);
tm_mat4 tm_translate(float x, float y, float z);
float tm_transform(tm_vec3 *v, tm_mat4 *t, float w);
tm_mat4 tm_rotate_xy(float x, float y);
tm_quat tm_from_axis_angle(float x, float y, float z, float theta);
tm_mat4 tm_quat_to_mat4(tm_quat *q);
void tm_normalize_vec4(tm_quat *a);
tm_quat tm_mult_quat(tm_quat *a, tm_quat *b);
void tm_normalize_quat(tm_quat *q);
tm_quat tm_slerp(tm_quat *a, tm_quat *b, float t);
tm_mat4 tm_mat4_identity();

#ifdef __cplusplus
}
#endif

#endif /* VECTOR_H */

/* 
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef BT_VECTOR
#define BT_VECTOR

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

typedef float bt_mat4[4][4];

extern int bt_dotvec3(bt_vec3 *a, bt_vec3 *b);
extern bt_vec3 bt_crossvec3(bt_vec3 *a, bt_vec3 *b);
extern void bt_rotate_into_vec(bt_vec3 *a, bt_vec3 *b);

#endif

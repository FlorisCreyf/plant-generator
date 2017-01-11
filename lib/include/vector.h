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

typedef struct TMvec3 {
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
} TMvec3;

typedef struct TMquat {
	union {
		struct {
			float x;
			float y;
			float z;
		};
		TMvec3 v;
	};
	float w;
} TMquat;

typedef struct TMmat4 {
	float m[4][4];
} TMmat4;

float absf(float f);
float tmDotVec3(TMvec3 *a, TMvec3 *b);
TMvec3 tmCrossVec3(TMvec3 *a, TMvec3 *b);
TMvec3 tmAddVec3(TMvec3 *a, TMvec3 *b);
TMvec3 tmSubVec3(TMvec3 *a, TMvec3 *b);
TMvec3 tmMultVec3(float a, TMvec3 *b);
float tmMagVec3(TMvec3 *a);
void tmNormalizeVec3(TMvec3 *a);
TMmat4 tmTransposeMat4(TMmat4 *m);
TMmat4 tmMultMat4(TMmat4 *a, TMmat4 *b);
TMmat4 tmRotateIntoVec(TMvec3 *normal, TMvec3 *direction);
TMvec3 tmRotateAroundAxis(TMvec3 *v, TMvec3 *axis, float n);
TMmat4 tmTranslate(float x, float y, float z);
float tmTransform(TMvec3 *v, TMmat4 *t, float w);
TMmat4 tmRotateXY(float x, float y);
TMquat tmFromAxisAngle(float x, float y, float z, float theta);
TMmat4 tmQuatToMat4(TMquat *q);
void tmNormalizeVec4(TMquat *a);
TMquat tmMultQuat(TMquat *a, TMquat *b);
void tmNormalizeQuat(TMquat *q);
TMquat tmSlerp(TMquat *a, TMquat *b, float t);
TMmat4 tmMat4Identity();

#ifdef __cplusplus
}
#endif

#endif /* VECTOR_H */

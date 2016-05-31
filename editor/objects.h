/* 
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef OBJECTS_H
#define OBJECTS_H

#include "vector.h"
#include <vector>

struct Mesh {
	int program;
	int attribs;
	int stride;
	std::vector<float> vertices;
	std::vector<float *> subMeshes;
	std::vector<unsigned short> triangles; 
};

struct Line {
	int program;
	int attribs;
	int stride;
	float width;
	std::vector<float> vertices;
};

#endif /* OBJECTS_H */

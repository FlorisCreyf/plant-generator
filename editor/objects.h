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

struct GeometryComponent
{
	int attribs;
	int stride;
	std::vector<float> vertices;
	std::vector<unsigned short> triangles;
	bool dynamic;
};

struct RenderComponent
{
	int program;
	int wireframeRange[2];
	int pointRange[2];
	int vertexRange[2];
	int triangleRange[2];
	enum Type {
		LINES, LINE_STRIP, LINE_LOOP, TRIANGLES
	} type;
};

struct Entity {
	RenderComponent renderComponent;
	GeometryComponent geometry;
};

#endif /* OBJECTS_H */

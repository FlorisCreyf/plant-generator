/* 
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "grid.h"
#include <iostream>
using namespace std;

Grid::Grid(int sections)
{
	const int VERTEX_COUNT = this->vertexCount = 8 * sections + 4;
	const int SIZE = VERTEX_COUNT * 3;
	const int Y = 0.0f;
	int i = 0;

	grid = new float[SIZE];
	
	grid[i++] = 0; grid[i++] = Y; grid[i++] =  sections;
	grid[i++] = 0; grid[i++] = Y; grid[i++] = -sections;
	grid[i++] =  sections; grid[i++] = Y; grid[i++] = 0;
	grid[i++] = -sections; grid[i++] = Y; grid[i++] = 0;

	for (int j = 1; j <= sections; j++) {
		grid[i++] =  j; grid[i++] = Y; grid[i++] =  sections;
		grid[i++] =  j; grid[i++] = Y; grid[i++] = -sections;
		grid[i++] = -j; grid[i++] = Y; grid[i++] =  sections;
		grid[i++] = -j; grid[i++] = Y; grid[i++] = -sections;

		grid[i++] =  sections; grid[i++] = Y; grid[i++] =  j;
		grid[i++] = -sections; grid[i++] = Y; grid[i++] =  j;
		grid[i++] =  sections; grid[i++] = Y; grid[i++] = -j;
		grid[i++] = -sections; grid[i++] = Y; grid[i++] = -j;
	}
}

Grid::~Grid()
{
	delete[] grid;
}

float *Grid::getBuffer() const
{
	return grid;
}

int Grid::getVertexCount() const
{
	return vertexCount;
}


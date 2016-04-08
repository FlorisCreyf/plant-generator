/* 
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef GRID_H
#define GRID_H

class Grid
{
public:
	Grid(int sections);
	~Grid();
		
	float *getBuffer() const;
	int getVertexCount() const;

private:
	float *grid;
	int vertexCount;
};

#endif /* GRID_H */

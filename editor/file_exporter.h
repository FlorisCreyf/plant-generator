/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef FILE_EXPORTER_H
#define FILE_EXPORTER_H

class FileExporter {
public:
	void setVertices(float *vertices, int size);
	void setTriangles(unsigned short *triangles, int size);
	void exportObj(const char *filename);

private:
	float* vb;
	int vbSize;
	unsigned short* eb;
	int ebSize;
};

#endif /* FILE_EXPORTER_H */

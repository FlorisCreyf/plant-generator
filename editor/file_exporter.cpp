/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "file_exporter.h"
#include <cstdio>

/* Interleaved buffer */
void FileExporter::setVertices(float *vertices, int size)
{
	this->vb = vertices;
	this->vbSize = size;
}

void FileExporter::setTriangles(unsigned short *triangles, int size)
{
	this->eb = triangles;
	this->ebSize = size;
}

void FileExporter::exportObj(const char *filename)
{
	FILE *f = fopen(filename, "w");

	for (int i = 0; i < vbSize * 6; i += 6) {
		fprintf(f, "v %f %f %f\n", vb[i], vb[i+1], vb[i+2]);
		fprintf(f, "vn %f %f %f\n", vb[i+3], vb[i+4], vb[i+5]);
	}

	for (int i = 0; i < ebSize; i += 3) {
		fprintf(f, "f %d %d %d\n", eb[i]+1, eb[i+1]+1, eb[i+2]+1);
	}

	fclose(f);
}

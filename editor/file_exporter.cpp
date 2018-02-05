/* Plant Genererator
 * Copyright (C) 2016-2017  Floris Creyf
 *
 * TreeMaker is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TreeMaker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "file_exporter.h"
#include <cstdio>

/* interleaved buffer */
void FileExporter::setVertices(const float *vertices, size_t size)
{
	this->vb = vertices;
	this->vbSize = size;
}

void FileExporter::setIndices(const unsigned *indices, size_t size)
{
	this->ib = indices;
	this->ibSize = size;
}

void FileExporter::exportObj(const char *filename)
{
	FILE *f = fopen(filename, "w");

	if (f == nullptr)
		return;

	for (size_t i = 0; i < vbSize; i += 6) {
		fprintf(f, "v %f %f %f\n", vb[i], vb[i+1], vb[i+2]);
		fprintf(f, "vn %f %f %f\n", vb[i+3], vb[i+4], vb[i+5]);
	}

	for (size_t i = 0; i < ibSize; i += 3) {
		fprintf(f, "f %d %d %d\n", ib[i]+1, ib[i+1]+1, ib[i+2]+1);
	}

	fclose(f);
}

/* Plant Genererator
 * Copyright (C) 2016-2018  Floris Creyf
 *
 * Plant Genererator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Plant Genererator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "file_exporter.h"
#include <cstdio>

void FileExporter::exportObj(const char *filename,
	const std::vector<float> &vertices,
	const std::vector<unsigned> &indices)
{
	FILE *f = fopen(filename, "w");

	if (f == nullptr)
		return;

	for (unsigned i = 0; i < vertices.size(); i += 8) {
		float x = vertices[i];
		float y = vertices[i+1];
		float z = vertices[i+2];
		fprintf(f, "v %f %f %f\n", x, y, z);
	}

	for (unsigned i = 0; i < vertices.size(); i += 8) {
		float u = vertices[i+6];
		float v = vertices[i+7];
		fprintf(f, "vt %f %f\n", u, v);
	}

	for (unsigned i = 0; i < vertices.size(); i += 8) {
		float x = vertices[i+3];
		float y = vertices[i+4];
		float z = vertices[i+5];
		fprintf(f, "vn %f %f %f\n", x, y, z);
	}

	for (size_t i = 0; i < indices.size(); i += 3)
		fprintf(f, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
			indices[i]+1, indices[i]+1, indices[i]+1,
			indices[i+1]+1, indices[i+1]+1, indices[i+1]+1,
			indices[i+2]+1, indices[i+2]+1, indices[i+2]+1);

	fclose(f);
}

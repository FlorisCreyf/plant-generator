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

#ifndef FILE_EXPORTER_H
#define FILE_EXPORTER_H

#include <cstddef>

class FileExporter {
public:
	void setVertices(const float *vertices, size_t size);
	void setIndices(const unsigned *indices, size_t size);
	void exportObj(const char *filename);

private:
	const float *vb;
	size_t vbSize;
	const unsigned *ib;
	size_t ibSize;
};

#endif /* FILE_EXPORTER_H */

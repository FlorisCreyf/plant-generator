/* TreeMaker: 3D tree model editor
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

#ifndef FILE_EXPORTER_H
#define FILE_EXPORTER_H

class FileExporter {
public:
	void setVertices(float *vertices, int size);
	void setTriangles(unsigned short *triangles, int size);
	void exportObj(const char *filename);

private:
	float *vb;
	int vbSize;
	unsigned short *eb;
	int ebSize;
};

#endif /* FILE_EXPORTER_H */

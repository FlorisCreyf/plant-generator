/* 
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef FILE_EXPORT_H
#define FILE_EXPORT_H

class FileExport
{
	public:
		FileExport();
		void setVertexBuffer(float *vb, int size);
		void setElementBuffer(unsigned short *eb, int size);
		void exportObj(const char *filename);

	private:
		float *vb;
		int vbSize;
		unsigned short *eb;
		int ebSize;
};

#endif /* FILE_EXPORT_H */

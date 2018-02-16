/* Plant Genererator
 * Copyright (C) 2018  Floris Creyf
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

#ifndef BUFFER_H
#define BUFFER_H

#define GL_GLEXT_PROTOTYPES

#include "../geometry/geometry.h"
#include <QOpenGLFunctions>

class Buffer : protected QOpenGLFunctions {
public:
	enum {Points = 0, Indices = 1};

	/** Creates and binds a new vertex array object. */
	void initialize(GLenum mode);
	void allocatePointMemory(int size);
	void allocateIndexMemory(int size);
	/**
	 * Allocates a new buffer.
	 * Important: Call initialize() or use() before calling load(...). The
	 * buffer can't be changed until the VAO is bound.
	 */
	void load(const Geometry &geometry);
	void load(const float *points, int psize, const unsigned *indices,
		int isize);
	/** Reduces reallocations and should be used for dynamic meshes. */
	void update(const Geometry &geometry);
	void update(const float *points, int psize, const unsigned *indices,
		int isize);
	/** Call before drawing. */
	void use();
	int getSize(int type);

private:
	GLuint vao;
	GLuint buffers[2];
	int size[2];
	int capacity[2];
	GLenum mode;

	void setVertexFormat();
};

#endif /* BUFFER_H */
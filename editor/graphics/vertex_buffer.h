/* Plant Generator
 * Copyright (C) 2020  Floris Creyf
 *
 * Plant Generator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Plant Generator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H

#include "editor/geometry/geometry.h"
#include "plant_generator/vertex.h"
#include <QOpenGLFunctions_4_3_Core>

class VertexBuffer : protected QOpenGLFunctions_4_3_Core {
public:
	enum {Points = 0, Indices = 1};

	/** Creates and binds a new vertex array object. */
	void initialize(GLenum mode);
	void allocatePointMemory(size_t size);
	void allocateIndexMemory(size_t size);
	/** Allocates a new buffer.
	IMPORTANT: Call initialize() or use() before calling load(...). The
	buffer can't be changed until the VAO is bound. */
	void load(const Geometry &geometry);
	void load(const pg::DVertex *points, size_t psize,
		const unsigned *indices, size_t isize);
	/** Reduces reallocations and should be used for dynamic meshes. */
	void update(const Geometry &geometry);
	void update(const pg::DVertex *points, size_t psize,
		const unsigned *indices, size_t isize);
	/** The buffer should be bound prior to calling update. The method
	returns false if the buffer is too small. */
	bool update(const pg::DVertex *points, size_t start, size_t size);
	/** The buffer should be bound prior to calling update. The method
	returns false if the buffer is too small. */
	bool update(const unsigned *indices, size_t start, size_t size);
	/** The buffer needs to be bound before drawing from it. */
	void use();
	size_t getSize(int type) const;
	size_t getCapacity(int type) const;

private:
	GLuint vao;
	GLuint buffers[2];
	size_t size[2];
	size_t capacity[2];
	GLenum mode;

	void setVertexFormat();
};

#endif

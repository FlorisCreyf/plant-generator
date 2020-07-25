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

#include "vertex_buffer.h"

using pg::DVertex;
using std::vector;

void VertexBuffer::initialize(GLenum mode)
{
	initializeOpenGLFunctions();
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(2, buffers);
	this->mode = mode;
}

void VertexBuffer::allocatePointMemory(size_t size)
{
	capacity[Points] = size;
	size *= sizeof(DVertex);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[Points]);
	glBufferData(GL_ARRAY_BUFFER, size, NULL, mode);
	setVertexFormat();
}

void VertexBuffer::allocateIndexMemory(size_t size)
{
	capacity[Indices] = size;
	size *= sizeof(unsigned);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[Indices]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, NULL, mode);
}

void VertexBuffer::load(const Geometry &geometry)
{
	const vector<DVertex> *p = geometry.getPoints();
	const vector<unsigned> *i = geometry.getIndices();
	load(p->data(), p->size(), i->data(), i->size());
}

void VertexBuffer::load(
	const DVertex *points, size_t psize,
	const unsigned *indices, size_t isize)
{
	size[Points] = capacity[Points] = psize;
	psize *= sizeof(DVertex);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[Points]);
	glBufferData(GL_ARRAY_BUFFER, psize, points, mode);
	setVertexFormat();

	if (isize > 0) {
		size[Indices] = capacity[Indices] = isize;
		isize *= sizeof(unsigned);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[Indices]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, isize, indices, mode);
	}
}

void VertexBuffer::update(const Geometry &geometry)
{
	const vector<DVertex> *p = geometry.getPoints();
	const vector<unsigned> *i = geometry.getIndices();
	update(p->data(), p->size(), i->data(), i->size());
}

void VertexBuffer::update(
	const DVertex *points, size_t psize,
	const unsigned *indices, size_t isize)
{
	use();
	size[Points] = psize;
	size[Indices] = isize;

	if (psize > capacity[Points])
		allocatePointMemory(psize * 2);
	else
		glBindBuffer(GL_ARRAY_BUFFER, buffers[Points]);

	psize *= sizeof(DVertex);
	isize *= sizeof(unsigned);

	glBufferSubData(GL_ARRAY_BUFFER, 0, psize, points);

	if (indices != nullptr) {
		if (isize > capacity[Indices])
			allocateIndexMemory(isize * 2);
		else
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[Indices]);

		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, isize, indices);
	}
}

bool VertexBuffer::update(const DVertex *points, size_t start, size_t size)
{
	size_t newSize = start + size;
	if (newSize <= capacity[Points])
		this->size[Points] = newSize;
	else
		return false;

	size *= sizeof(DVertex);
	start *= sizeof(DVertex);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[Points]);
	glBufferSubData(GL_ARRAY_BUFFER, start, size, points);
	return true;
}

bool VertexBuffer::update(const unsigned *indices, size_t start, size_t size)
{
	size_t newSize = start + size;
	if (newSize <= capacity[Indices])
		this->size[Indices] = newSize;
	else
		return false;

	size *= sizeof(unsigned);
	start *= sizeof(unsigned);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[Indices]);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, start, size, indices);
	return true;
}

void VertexBuffer::setVertexFormat()
{
	GLsizei stride = sizeof(DVertex);
	GLvoid *ptr = (GLvoid *)(offsetof(DVertex, position));
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, ptr);
	glEnableVertexAttribArray(0);
	ptr = (GLvoid *)(offsetof(DVertex, normal));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, ptr);
	glEnableVertexAttribArray(1);
	ptr = (GLvoid *)(offsetof(DVertex, uv));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, ptr);
	glEnableVertexAttribArray(2);
	ptr = (GLvoid *)(offsetof(DVertex, indices));
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride, ptr);
	glEnableVertexAttribArray(3);
	ptr = (GLvoid *)(offsetof(DVertex, weights));
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, stride, ptr);
	glEnableVertexAttribArray(4);
}

void VertexBuffer::use()
{
	glBindVertexArray(vao);
}

size_t VertexBuffer::getSize(int type) const
{
	return size[type];
}

size_t VertexBuffer::getCapacity(int type) const
{
	return capacity[type];
}

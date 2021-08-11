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
	glGenVertexArrays(1, &this->vao);
	glBindVertexArray(this->vao);
	glGenBuffers(2, this->buffers);
	this->mode = mode;
}

void VertexBuffer::allocatePointMemory(size_t size)
{
	this->capacity[Points] = size;
	size *= sizeof(DVertex);
	glBindBuffer(GL_ARRAY_BUFFER, this->buffers[Points]);
	glBufferData(GL_ARRAY_BUFFER, size, NULL, this->mode);
	setVertexFormat();
}

void VertexBuffer::allocateIndexMemory(size_t size)
{
	this->capacity[Indices] = size;
	size *= sizeof(unsigned);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->buffers[Indices]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, NULL, this->mode);
}

void VertexBuffer::load(const Geometry &geometry)
{
	const vector<DVertex> *p = geometry.getPoints();
	const vector<unsigned> *i = geometry.getIndices();
	load(p->data(), p->size(), i->data(), i->size());
}

void VertexBuffer::load(const DVertex *points, size_t psize,
	const unsigned *indices, size_t isize)
{
	this->size[Points] = this->capacity[Points] = psize;
	psize *= sizeof(DVertex);
	glBindBuffer(GL_ARRAY_BUFFER, this->buffers[Points]);
	glBufferData(GL_ARRAY_BUFFER, psize, points, this->mode);
	setVertexFormat();

	if (isize > 0) {
		this->size[Indices] = this->capacity[Indices] = isize;
		isize *= sizeof(unsigned);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->buffers[Indices]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, isize, indices,
			this->mode);
	}
}

void VertexBuffer::update(const Geometry &geometry)
{
	const vector<DVertex> *p = geometry.getPoints();
	const vector<unsigned> *i = geometry.getIndices();
	update(p->data(), p->size(), i->data(), i->size());
}

void VertexBuffer::update(const DVertex *points, size_t psize,
	const unsigned *indices, size_t isize)
{
	glBindVertexArray(this->vao);
	this->size[Points] = psize;
	this->size[Indices] = isize;

	if (psize > this->capacity[Points])
		allocatePointMemory(psize * 2);
	else
		glBindBuffer(GL_ARRAY_BUFFER, this->buffers[Points]);
	psize *= sizeof(DVertex);
	glBufferSubData(GL_ARRAY_BUFFER, 0, psize, points);

	if (!indices)
		return;

	if (isize > this->capacity[Indices])
		allocateIndexMemory(isize * 2);
	else
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->buffers[Indices]);
	isize *= sizeof(unsigned);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, isize, indices);
}

bool VertexBuffer::update(const DVertex *points, size_t start, size_t size)
{
	size_t newSize = start + size;
	if (newSize <= this->capacity[Points])
		this->size[Points] = newSize;
	else
		return false;

	size *= sizeof(DVertex);
	start *= sizeof(DVertex);
	glBindBuffer(GL_ARRAY_BUFFER, this->buffers[Points]);
	glBufferSubData(GL_ARRAY_BUFFER, start, size, points);
	return true;
}

bool VertexBuffer::update(const unsigned *indices, size_t start, size_t size)
{
	size_t newSize = start + size;
	if (newSize <= this->capacity[Indices])
		this->size[Indices] = newSize;
	else
		return false;

	size *= sizeof(unsigned);
	start *= sizeof(unsigned);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->buffers[Indices]);
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
	ptr = (GLvoid *)(offsetof(DVertex, tangent));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, ptr);
	glEnableVertexAttribArray(2);
	ptr = (GLvoid *)(offsetof(DVertex, tangentScale));
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, stride, ptr);
	glEnableVertexAttribArray(3);
	ptr = (GLvoid *)(offsetof(DVertex, uv));
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, stride, ptr);
	glEnableVertexAttribArray(4);
	ptr = (GLvoid *)(offsetof(DVertex, indices));
	glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, stride, ptr);
	glEnableVertexAttribArray(5);
	ptr = (GLvoid *)(offsetof(DVertex, weights));
	glVertexAttribPointer(6, 2, GL_FLOAT, GL_FALSE, stride, ptr);
	glEnableVertexAttribArray(6);
}

void VertexBuffer::use()
{
	glBindVertexArray(this->vao);
}

size_t VertexBuffer::getSize(int type) const
{
	return this->size[type];
}

size_t VertexBuffer::getCapacity(int type) const
{
	return this->capacity[type];
}

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

#include "storage_buffer.h"


StorageBuffer::StorageBuffer() : size(0)
{

}

void StorageBuffer::initialize(GLenum usage, GLuint index)
{
	initializeOpenGLFunctions();
	this->usage = usage;
	this->index = index;
	glGenBuffers(1, &this->ssbo);
}

void StorageBuffer::load(const void *data, size_t size)
{
	this->size = size;
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, this->usage);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, this->index, this->ssbo);

}

bool StorageBuffer::update(const void *data, size_t size)
{
	if (size > this->size)
		return false;
	else {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->ssbo);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, size, data);
		return true;
	}
}

void StorageBuffer::bind()
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, this->index, this->ssbo);
}

size_t StorageBuffer::getSize() const
{
	return this->size;
}

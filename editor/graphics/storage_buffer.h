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

#ifndef STORAGE_BUFFER_H
#define SOTRAGE_BUFFER_H

#include <QOpenGLFunctions_4_3_Core>

class StorageBuffer : protected QOpenGLFunctions_4_3_Core {
	GLuint ssbo;
	GLenum usage;
	GLuint index;
	size_t size;

public:
	StorageBuffer();
	void initialize(GLenum usage, GLuint index);
	void load(const void *data, size_t size);
	bool update(const void *data, size_t size);
	void bind();
	size_t getSize() const;
};

#endif /* STORAGE_BUFFER_H */

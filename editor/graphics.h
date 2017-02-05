/* TreeMaker: 3D tree model editor
 * Copyright (C) 2017  Floris Creyf
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

#ifndef GRAPHICS_H
#define GRAPHICS_H

#define GL_GLEXT_PROTOTYPES

#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <vector>

namespace graphics {
	enum VertexFormat {VERTEX_COLOR, VERTEX_NORMAL, VERTEX_NORMAL_TEX};
	struct Fragment {
		GLint start[2];
		GLsizei count[2];
		GLenum type;
	};
	struct BufferSet {
		GLuint vao;
		GLuint buffers[2];
		GLsizei sizes[2];
	};

	GLuint buildShader(GLenum type, const char *filename);
	GLuint buildProgram(GLuint *shaders, int size);
	void setVertexFormat(VertexFormat format);
	int getSize(VertexFormat format);

	template <typename T>
	void load(GLenum target, std::vector<T> data, GLenum usage);

	template <typename T>
	void update(GLenum target, int index, std::vector<T> data);
}

template <typename T>
void graphics::load(GLenum target, std::vector<T> data, GLenum usage)
{
	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
	GLsizei size = sizeof(T) * data.size();
	f->glBufferData(target, size, &data[0], usage);
}

template <typename T>
void graphics::update(GLenum target, int index, std::vector<T> data)
{
	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
	GLsizeiptr size = sizeof(T) * data.size();
	GLintptr offset = sizeof(T) * index;
	f->glBufferSubData(target, offset, size, &data[0]);
}

#endif /* GRAPHICS_H */

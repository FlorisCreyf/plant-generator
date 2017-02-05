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

#include "graphics.h"
#include "terminal.h"
#include <stdio.h>

bool isCompiled(GLuint name, const char *filename)
{
	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

	GLint status;
	f->glGetShaderiv(name, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE) {
		GLsizei size;
		GLchar *log;

		f->glGetShaderiv(name, GL_INFO_LOG_LENGTH, &size);
		log = new GLchar[size + 1];
		f->glGetShaderInfoLog(name, size, &size, log);
		fprintf(stderr, BOLDWHITE "%s: " BOLDRED "error:" RESET "\n%s",
				filename, log);

		delete[] log;
		return false;
	}

	return true;
}

bool openFile(const char *filename, GLchar *&buffer, int &size)
{
	FILE *file = fopen(filename, "r");

	if (file == NULL) {
		fprintf(stderr, BOLDMAGENTA "warning:" RESET" %s not found\n",
				filename);
		return false;
	}

	fseek(file, 0, SEEK_END);
	size = ftell(file);
	rewind(file);

	buffer = new GLchar[size];
	if (!fread(buffer, 1, size, file)) {
		fprintf(stderr, BOLDMAGENTA "warning:" RESET
				"failed to read %s\n", filename);
		delete[] buffer;
		return false;
	}

	fclose(file);
	return true;
}

GLuint graphics::buildShader(GLenum type, const char *filename)
{
	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

	GLuint shader;
	GLchar *buffer;
	int size;

	if(!openFile(filename, buffer, size))
		return 0;

	shader = f->glCreateShader(type);
	f->glShaderSource(shader, 1, (const char**)&buffer, &size);
	f->glCompileShader(shader);

	if (!isCompiled(shader, filename)) {
		glDeleteShader(shader);
		return 0;
	}

	return shader;
}

GLuint graphics::buildProgram(GLuint *shaders, int size)
{
	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

	GLuint program = f->glCreateProgram();

	for (int i = 0; i < size; i++)
		if (shaders[i] != 0)
			f->glAttachShader(program, shaders[i]);

	f->glLinkProgram(program);
	return program;
}

void graphics::setVertexFormat(VertexFormat format)
{
	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

	int vectors = 0;
	GLvoid *ptr = (void *)0;
	GLsizei stride;

	switch (format) {
	case VERTEX_NORMAL:
	case VERTEX_COLOR:
		vectors = 2;
		stride = sizeof(float) * 6;
		break;
	default:
		break;
	}

	for (int i = 0; i < vectors; i++) {
		ptr = (GLvoid *)(sizeof(float) * 3 * i);
		f->glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, stride, ptr);
		f->glEnableVertexAttribArray(i);
	}
}

int graphics::getSize(VertexFormat format)
{
	static int sizes[] = {6, 6, 8};
	return sizes[format];
}

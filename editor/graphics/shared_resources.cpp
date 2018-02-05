/* Plant Genererator
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

#include "shared_resources.h"
#include "../terminal.h"
#include <QImage>
#include <stdio.h>

void SharedResources::initialize()
{
	initializeOpenGLFunctions();
	createPrograms();
	createTextures();
}

void SharedResources::createPrograms()
{
	GLuint vertModel = buildShader(GL_VERTEX_SHADER,
			"shaders/basic.vert");
	GLuint fragModel = buildShader(GL_FRAGMENT_SHADER,
			"shaders/basic.frag");
	GLuint vertFlat = buildShader(GL_VERTEX_SHADER,
			"shaders/flat.vert");
	GLuint fragFlat = buildShader(GL_FRAGMENT_SHADER,
			"shaders/flat.frag");
	GLuint vertWireframe = buildShader(GL_VERTEX_SHADER,
			"shaders/solid.vert");
	GLuint fragPoint = buildShader(GL_FRAGMENT_SHADER,
			"shaders/point.frag");
	GLuint vertLine = buildShader(GL_VERTEX_SHADER,
			"shaders/line.vert");
	GLuint geomLine = buildShader(GL_GEOMETRY_SHADER,
			"shaders/line.geom");
	GLuint fragLine = buildShader(GL_FRAGMENT_SHADER,
			"shaders/line.frag");

	GLuint shaders[3];
	GLuint program;

	shaders[0] = vertModel;
	shaders[1] = fragModel;
	program = buildProgram(shaders, 2);
	programs[Shader::Model] = program;

	shaders[0] = vertFlat;
	shaders[1] = fragFlat;
	program = buildProgram(shaders, 2);
	programs[Shader::Flat] = program;

	shaders[0] = vertWireframe;
	shaders[1] = fragFlat;
	program = buildProgram(shaders, 2);
	programs[Shader::Wire] = program;

	shaders[0] = vertFlat;
	shaders[1] = fragPoint;
	program = buildProgram(shaders, 2);
	programs[Shader::Point] = program;

	shaders[0] = vertLine;
	shaders[1] = geomLine;
	shaders[2] = fragLine;
	program = buildProgram(shaders, 3);
	programs[Shader::Line] = program;
}

void SharedResources::createTextures()
{
	QImage image("resources/dot.png");
	image.convertToFormat(QImage::Format_RGB32);
	GLsizei width = image.width();
	GLsizei height = image.height();

	glGenTextures(1, &textures[DotTexture]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[DotTexture]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_BGRA,
			GL_UNSIGNED_BYTE, image.bits());
}

GLuint SharedResources::getShader(Shader shader)
{
	return programs[shader];
}

GLuint SharedResources::getTexture(Texture texture)
{
	return textures[texture];
}

bool SharedResources::isCompiled(GLuint name, const char *filename)
{
	GLint status;
	glGetShaderiv(name, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE) {
		GLsizei size;
		GLchar *log;

		glGetShaderiv(name, GL_INFO_LOG_LENGTH, &size);
		log = new GLchar[size + 1];
		glGetShaderInfoLog(name, size, &size, log);
		fprintf(stderr, BOLDWHITE "%s: " BOLDRED "error:" RESET "\n%s",
			filename, log);

		delete[] log;
		return false;
	}

	return true;
}

bool SharedResources::openFile(const char *filename, GLchar *&buffer, int &size)
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

GLuint SharedResources::buildShader(GLenum type, const char *filename)
{
	GLuint shader;
	GLchar *buffer;
	int size;

	if(!openFile(filename, buffer, size))
		return 0;

	shader = glCreateShader(type);
	glShaderSource(shader, 1, (const char**)&buffer, &size);
	glCompileShader(shader);

	if (!isCompiled(shader, filename)) {
		glDeleteShader(shader);
		return 0;
	}

	return shader;
}

GLuint SharedResources::buildProgram(GLuint *shaders, int size)
{
	GLuint program = glCreateProgram();

	for (int i = 0; i < size; i++)
		if (shaders[i] != 0)
			glAttachShader(program, shaders[i]);

	glLinkProgram(program);
	return program;
}

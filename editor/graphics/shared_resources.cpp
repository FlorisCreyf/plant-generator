/* Plant Genererator
 * Copyright (C) 2017  Floris Creyf
 *
 * Plant Genererator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Plant Genererator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "shared_resources.h"
#include "editor/terminal.h"
#include <QImage>
#include <stdio.h>

void SharedResources::initialize()
{
	if (!initialized) {
		initializeOpenGLFunctions();
		createPrograms();

		QImage image1("resources/dot.png");
		textures[DotTexture] = addTexture(image1);
		QImage image2("resources/default.png");
		textures[DefaultTexture] = addTexture(image2);

		defaultMaterial.setDefaultTexture(0, textures[DefaultTexture]);
		materials[0] = defaultMaterial;

		initialized = true;
	}
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
	GLuint fragMaterial = buildShader(GL_FRAGMENT_SHADER,
		"shaders/material.frag");
	GLuint vertOutline = buildShader(GL_VERTEX_SHADER,
		"shaders/outline.vert");
	GLuint fragOutline = buildShader(GL_FRAGMENT_SHADER,
		"shaders/outline.frag");

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

	shaders[0] = vertFlat;
	shaders[1] = fragMaterial;
	program = buildProgram(shaders, 2);
	programs[Shader::Material] = program;

	shaders[0] = vertOutline;
	shaders[1] = fragOutline;
	program = buildProgram(shaders, 2);
	programs[Shader::Outline] = program;
}

GLuint SharedResources::addTexture(const QImage &image)
{
	GLuint name;
	GLsizei width = image.width();
	GLsizei height = image.height();

	glGenTextures(1, &name);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, name);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_BGRA,
		GL_UNSIGNED_BYTE, image.bits());

	return name;
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

		glGetShaderiv(name, GL_INFO_LOG_LENGTH, &size);
		GLchar *log = new GLchar[size + 1];
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
	int size;
	GLchar *buffer;
	if(!openFile(filename, buffer, size))
		return 0;

	GLuint shader = glCreateShader(type);
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

void SharedResources::addMaterial(ShaderParams params)
{
	auto it = materials.find(params.getID());
	if (it != materials.end()) {
		QString before = QString::fromStdString(it->second.getName());
		QString after = QString::fromStdString(params.getName());
		materials[params.getID()] = params;
		if (before != after)
			emit materialRenamed(before, after);
		emit materialModified(params);
	} else
		materials[params.getID()] = params;

	emit materialAdded(params);
}

void SharedResources::removeMaterial(long id)
{
	auto it = materials.find(id);
	QString name = QString::fromStdString(it->second.getName());
	it->second.clearTextures();
	materials.erase(it);
	emit materialRemoved(name);
}

void SharedResources::clearMaterials()
{
	for (auto it = materials.begin(); it != materials.end(); it++) {
		QString name = QString::fromStdString(it->second.getName());
		it->second.clearTextures();
		emit materialRemoved(name);
	}
	materials.clear();
	materials[0] = defaultMaterial;
}

ShaderParams SharedResources::getMaterial(long id)
{
	return materials[id];
}

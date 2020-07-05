/* Plant Generator
 * Copyright (C) 2017  Floris Creyf
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

#include "shared_resources.h"
#include "editor/terminal.h"
#include <QImage>
#include <fstream>
#include <sstream>
#include <iostream>

SharedResources::SharedResources()
{
	initialized = false;
	textures[0] = textures[1] = 0;
}

void SharedResources::initialize()
{
	if (!initialized) {
		initializeOpenGLFunctions();
		createPrograms();

		QImage image1("resources/dot.png");
		textures[DotTexture] = addTexture(image1);
		QImage image2("resources/default.png");
		textures[DefaultTexture] = addTexture(image2);

		for (ShaderParams &param : materials)
			if (!param.isValid()) {
				GLuint tex = textures[DefaultTexture];
				param.setDefaultTexture(0, tex);
				param.initialize();
			}

		surface.create();
		initialized = true;
	}
}

void SharedResources::createPrograms()
{
	GLuint vertSolid = buildShader(GL_VERTEX_SHADER,
		"shaders/model.vert", "#define SOLID\n");
	GLuint fragSolid = buildShader(GL_FRAGMENT_SHADER,
		"shaders/model.frag", "#define SOLID\n");
	GLuint vertWireframe = buildShader(GL_VERTEX_SHADER,
		"shaders/model.vert", "#define WIREFRAME\n");
	GLuint fragWireframe = buildShader(GL_FRAGMENT_SHADER,
		"shaders/model.frag", "#define WIREFRAME\n");
	GLuint vertMaterial = buildShader(GL_VERTEX_SHADER,
		"shaders/model.vert", "#define MATERIAL\n");
	GLuint fragMaterial = buildShader(GL_FRAGMENT_SHADER,
		"shaders/model.frag", "#define MATERIAL\n");
	GLuint vertFlat = buildShader(GL_VERTEX_SHADER,
		"shaders/flat.vert", nullptr);
	GLuint fragFlat = buildShader(GL_FRAGMENT_SHADER,
		"shaders/flat.frag", nullptr);
	GLuint fragPoint = buildShader(GL_FRAGMENT_SHADER,
		"shaders/point.frag", nullptr);
	GLuint vertLine = buildShader(GL_VERTEX_SHADER,
		"shaders/line.vert", nullptr);
	GLuint geomLine = buildShader(GL_GEOMETRY_SHADER,
		"shaders/line.geom", nullptr);
	GLuint fragLine = buildShader(GL_FRAGMENT_SHADER,
		"shaders/line.frag", nullptr);
	GLuint vertOutline = buildShader(GL_VERTEX_SHADER,
		"shaders/outline.vert", nullptr);
	GLuint fragOutline = buildShader(GL_FRAGMENT_SHADER,
		"shaders/outline.frag", nullptr);

	GLuint shaders[3];
	shaders[0] = vertSolid;
	shaders[1] = fragSolid;
	programs[Shader::Solid] = buildProgram(shaders, 2);
	shaders[0] = vertWireframe;
	shaders[1] = fragWireframe;
	programs[Shader::Wireframe] = buildProgram(shaders, 2);
	shaders[0] = vertMaterial;
	shaders[1] = fragMaterial;
	programs[Shader::Material] = buildProgram(shaders, 2);
	shaders[0] = vertFlat;
	shaders[1] = fragFlat;
	programs[Shader::Flat] = buildProgram(shaders, 2);
	shaders[0] = vertFlat;
	shaders[1] = fragPoint;
	programs[Shader::Point] = buildProgram(shaders, 2);
	shaders[0] = vertLine;
	shaders[1] = geomLine;
	shaders[2] = fragLine;
	programs[Shader::Line] = buildProgram(shaders, 3);
	shaders[0] = vertOutline;
	shaders[1] = fragOutline;
	programs[Shader::Outline] = buildProgram(shaders, 2);
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
	glTexSubImage2D(
		GL_TEXTURE_2D, 0, 0, 0, width, height, GL_BGRA,
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
		std::cerr << BOLDWHITE << filename << ": ";
		std::cerr << BOLDRED "error:" RESET "\n" << log << std::endl;
		delete[] log;
		return false;
	}
	return true;
}

bool SharedResources::openFile(const char *filename, std::string &buffer)
{
	std::ifstream file(filename, std::ios::in);
	if (!file.is_open()) {
		std::cerr << BOLDMAGENTA "warning: " RESET << filename;
		std::cerr << " not found" << std::endl;
		return false;
	}
	std::stringstream stream;
	stream << file.rdbuf();
	buffer = stream.str();
	return true;
}

GLuint SharedResources::buildShader(
	GLenum type, const char *filename, const char *line)
{
	std::string buffer;
	if(!openFile(filename, buffer))
		return 0;

	if (line) {
		size_t offset = buffer.find("\n") + 1;
		buffer.insert(offset, line);
	}

	GLuint shader = glCreateShader(type);
	const GLint size = static_cast<GLint>(buffer.size());
	const GLchar *glBuffer = buffer.c_str();
	glShaderSource(shader, 1, &glBuffer, &size);
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

unsigned SharedResources::addMaterial(ShaderParams params)
{
	params.setDefaultTexture(0, textures[DefaultTexture]);
	materials.push_back(params);
	emit materialAdded(params);
	return materials.size()-1;
}

void SharedResources::updateMaterial(ShaderParams params, unsigned index)
{
	materials[index] = params;
	emit materialModified(index);
}

void SharedResources::removeMaterial(unsigned index)
{
	auto context = QOpenGLContext::globalShareContext();
	context->makeCurrent(&this->surface);
	materials[index].clearTextures();
	materials.erase(materials.begin()+index);
	emit materialRemoved(index);
	context->doneCurrent();
}

void SharedResources::clearMaterials()
{
	auto context = QOpenGLContext::globalShareContext();
	context->makeCurrent(&this->surface);
	for (size_t i = 0; i < materials.size(); i++) {
		materials[i].clearTextures();
		emit materialRemoved(i);
	}
	context->doneCurrent();
	materials.clear();
}

ShaderParams SharedResources::getMaterial(unsigned index) const
{
	return materials[index];
}

unsigned SharedResources::getMaterialCount() const
{
	return materials.size();
}

QOffscreenSurface *SharedResources::getSurface()
{
	return &this->surface;
}

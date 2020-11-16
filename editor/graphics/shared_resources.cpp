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

using pg::Material;

SharedResources::SharedResources()
{
	this->initialized = false;
	for (int i = 0; i < TextureQuantity; i++)
		this->textures[i] = 0;
}

void SharedResources::initialize()
{
	if (!this->initialized) {
		initializeOpenGLFunctions();
		createPrograms();

		QImage img("resources/dot.png");
		this->textures[DotTexture] = addTexture(img, GL_RGBA, GL_RGBA8);
		unsigned char black[3] = {0, 0, 0};
		unsigned char white[3] = {255, 255, 255};
		unsigned char blue[3] = {127, 127, 255};
		this->textures[BlackTexture] = addDefaultTexture(black);
		this->textures[WhiteTexture] = addDefaultTexture(white);
		this->textures[BlueTexture] = addDefaultTexture(blue);

		for (ShaderParams &param : this->materials)
			if (!param.isValid()) {
				param.setDefaultTexture(Material::Albedo,
					this->textures[BlackTexture]);
				param.setDefaultTexture(Material::Opacity,
					this->textures[WhiteTexture]);
				param.setDefaultTexture(Material::Normal,
					this->textures[BlueTexture]);
				param.setDefaultTexture(Material::Specular,
					this->textures[WhiteTexture]);
				param.initialize();
			}

		this->surface.create();
		this->initialized = true;
	}
}

void SharedResources::createPrograms()
{
	GLuint shaders[3];

	GLuint solidFS = buildShader(GL_FRAGMENT_SHADER,
		"shaders/model.frag", "#define SOLID\n");
	GLuint wireframeFS = buildShader(GL_FRAGMENT_SHADER,
		"shaders/model.frag", "#define WIREFRAME\n");
	GLuint materialFS = buildShader(GL_FRAGMENT_SHADER,
		"shaders/model.frag", "#define MATERIAL\n");
	GLuint outlineFS = buildShader(GL_FRAGMENT_SHADER,
		"shaders/model.frag", "#define OUTLINE\n");

	shaders[0] = buildShader(GL_VERTEX_SHADER,
		"shaders/model.vert", "#define SOLID\n");
	shaders[1] = solidFS;
	this->programs[Shader::Solid] = buildProgram(shaders, 2);
	shaders[0] = buildShader(GL_VERTEX_SHADER,
		"shaders/model.vert", "#define WIREFRAME\n");
	shaders[1] = wireframeFS;
	this->programs[Shader::Wireframe] = buildProgram(shaders, 2);
	shaders[0] = buildShader(GL_VERTEX_SHADER,
		"shaders/model.vert", "#define MATERIAL\n");
	shaders[1] = materialFS;
	this->programs[Shader::Material] = buildProgram(shaders, 2);
	shaders[0] = buildShader(GL_VERTEX_SHADER,
		"shaders/model.vert", "#define OUTLINE\n");
	shaders[1] = outlineFS;
	this->programs[Shader::Outline] = buildProgram(shaders, 2);

	shaders[0] = buildShader(GL_VERTEX_SHADER,
		"shaders/model.vert", "#define SOLID\n#define DYNAMIC\n");
	shaders[1] = solidFS;
	this->programs[Shader::DynamicSolid] = buildProgram(shaders, 2);
	shaders[0] = buildShader(GL_VERTEX_SHADER,
		"shaders/model.vert", "#define WIREFRAME\n#define DYNAMIC\n");
	shaders[1] = wireframeFS;
	this->programs[Shader::DynamicWireframe] = buildProgram(shaders, 2);
	shaders[0] = buildShader(GL_VERTEX_SHADER,
		"shaders/model.vert", "#define MATERIAL\n#define DYNAMIC\n");
	shaders[1] = materialFS;
	this->programs[Shader::DynamicMaterial] = buildProgram(shaders, 2);
	shaders[0] = buildShader(GL_VERTEX_SHADER,
		"shaders/model.vert", "#define OUTLINE\n#define DYNAMIC\n");
	shaders[1] = outlineFS;
	this->programs[Shader::DynamicOutline] = buildProgram(shaders, 2);

	shaders[0] = buildShader(GL_VERTEX_SHADER,
		"shaders/line.vert", nullptr);
	shaders[1] = buildShader(GL_GEOMETRY_SHADER,
		"shaders/line.geom", nullptr);
	shaders[2] = buildShader(GL_FRAGMENT_SHADER,
		"shaders/line.frag", nullptr);
	this->programs[Shader::Line] = buildProgram(shaders, 3);

	GLuint flatVS = buildShader(GL_VERTEX_SHADER,
		"shaders/flat.vert", nullptr);

	shaders[0] = flatVS;
	shaders[1] = buildShader(GL_FRAGMENT_SHADER,
		"shaders/flat.frag", nullptr);
	this->programs[Shader::Flat] = buildProgram(shaders, 2);
	shaders[0] = flatVS;
	shaders[1] = buildShader(GL_FRAGMENT_SHADER,
		"shaders/point.frag", nullptr);
	this->programs[Shader::Point] = buildProgram(shaders, 2);
}

GLuint SharedResources::addTexture(
	const QImage &image, GLenum baseformat, GLenum sizeformat)
{
	GLuint name;
	GLsizei width = image.width();
	GLsizei height = image.height();
	glGenTextures(1, &name);
	glBindTexture(GL_TEXTURE_2D, name);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexStorage2D(GL_TEXTURE_2D, 1, sizeformat, width, height);
	glTexSubImage2D(
		GL_TEXTURE_2D, 0, 0, 0, width, height, baseformat,
		GL_UNSIGNED_BYTE, image.bits());
	return name;
}

GLuint SharedResources::addDefaultTexture(const unsigned char color[3])
{
	GLuint name;
	GLsizei width = 1;
	GLsizei height = 1;
	glGenTextures(1, &name);
	glBindTexture(GL_TEXTURE_2D, name);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, width, height);
	glTexSubImage2D(
		GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB,
		GL_UNSIGNED_BYTE, color);
	return name;
}

GLuint SharedResources::getShader(Shader shader)
{
	return this->programs[shader];
}

GLuint SharedResources::getTexture(Texture texture)
{
	return this->textures[texture];
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
	params.setDefaultTexture(
		Material::Albedo, this->textures[BlackTexture]);
	params.setDefaultTexture(
		Material::Opacity, this->textures[WhiteTexture]);
	params.setDefaultTexture(
		Material::Normal, this->textures[BlueTexture]);
	params.setDefaultTexture(
		Material::Specular, this->textures[WhiteTexture]);
	this->materials.push_back(params);
	emit materialAdded(params);
	return this->materials.size()-1;
}

void SharedResources::updateMaterial(ShaderParams params, unsigned index)
{
	this->materials[index] = params;
	emit materialModified(index);
}

void SharedResources::removeMaterial(unsigned index)
{
	auto context = QOpenGLContext::globalShareContext();
	context->makeCurrent(&this->surface);
	this->materials[index].clearTextures();
	this->materials.erase(this->materials.begin()+index);
	emit materialRemoved(index);
	context->doneCurrent();
}

void SharedResources::clearMaterials()
{
	auto context = QOpenGLContext::globalShareContext();
	context->makeCurrent(&this->surface);
	for (size_t i = 0; i < this->materials.size(); i++) {
		this->materials[i].clearTextures();
		emit materialRemoved(i);
	}
	context->doneCurrent();
	this->materials.clear();
}

ShaderParams SharedResources::getMaterial(unsigned index)
{
	this->materials[index].initialize();
	return this->materials.at(index);
}

unsigned SharedResources::getMaterialCount() const
{
	return this->materials.size();
}

QOffscreenSurface *SharedResources::getSurface()
{
	return &this->surface;
}

/* Plant Generator
 * Copyright (C) 2019  Floris Creyf
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

#include "shader_params.h"
#include <QImage>
#include <QImageReader>
#include <QOpenGLFunctions_4_3_Core>

using pg::Material;

ShaderParams::ShaderParams()
{

}

ShaderParams::ShaderParams(Material material) : material(material)
{
	QString filename;
	for (int i = 0; i < Material::MapQuantity; i++) {
		filename = QString::fromStdString(material.getTexture(i));
		this->textureFiles[i] = filename;
		loadTexture(i, filename);
	}
}

void ShaderParams::setName(std::string name)
{
	material.setName(name);
}

std::string ShaderParams::getName()
{
	return material.getName();
}

void ShaderParams::swapMaterial(Material material)
{
	this->material = material;
}

Material ShaderParams::getMaterial()
{
	return material;
}

GLuint ShaderParams::getTexture(int index)
{
	return textures[index];
}

bool ShaderParams::isValid() const
{
	return this->valid;
}

void ShaderParams::initialize()
{
	if (!this->valid && QOpenGLContext::currentContext()) {
		this->valid = true;
		for (int i = 0; i < Material::MapQuantity; i++)
			if (!this->textureFiles[i].isEmpty())
				loadTexture(i, this->textureFiles[i]);
	}
}

bool ShaderParams::loadTexture(int index, QString filename)
{
	this->textureFiles[index] = filename;
	if (QOpenGLContext::currentContext())
		this->valid = true;
	else {
		this->valid = false;
		return false;
	}

	QImageReader reader(filename);
	reader.setAutoTransform(true);
	QImage image = reader.read();
	if (!image.isNull()) {
		image = image.mirrored(false, true);
		if (!material.getTexture(index).empty())
			removeTexture(index);
		float ratio = (float)image.width() / (float)image.height();
		material.setRatio(ratio);
		material.setTexture(filename.toStdString(), index);
		this->textures[index] = loadTexture(image);
		return true;
	} else
		return false;
}

GLuint ShaderParams::loadTexture(QImage image)
{
	GLuint name;
	GLsizei width = image.width();
	GLsizei height = image.height();
	auto context = QOpenGLContext::globalShareContext();
	auto f = context->extraFunctions();
	f->glGenTextures(1, &name);
	f->glBindTexture(GL_TEXTURE_2D, name);
	f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	f->glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
	f->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_BGRA,
		GL_UNSIGNED_BYTE, image.bits());
	return name;
}

void ShaderParams::removeTexture(int index)
{
	auto context = QOpenGLContext::globalShareContext();
	auto f = context->extraFunctions();
	material.setRatio(1.0f);
	material.setTexture("", index);
	bool exists = this->textures[index] != 0;
	bool isDefault = this->textures[index] == this->defaultTextures[index];
	if (exists && !isDefault)
		f->glDeleteTextures(1, &this->textures[index]);
	this->textures[index] = this->defaultTextures[index];
}

void ShaderParams::clearTextures()
{
	for (int i = 0; i < Material::MapQuantity; i++)
		removeTexture(i);
}

void ShaderParams::setDefaultTexture(int index, GLuint name)
{
	this->defaultTextures[index] = name;
	if (this->textures[index] == 0)
		this->textures[index] = name;
}

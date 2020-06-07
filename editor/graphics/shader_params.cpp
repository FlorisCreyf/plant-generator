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
#include <QOpenGLExtraFunctions>

ShaderParams::ShaderParams()
{

}

ShaderParams::ShaderParams(pg::Material material) : material(material)
{
	QString filename = QString::fromStdString(material.getTexture());
	loadTexture(0, filename);
}

long ShaderParams::getID()
{
	return material.getID();
}

void ShaderParams::setName(std::string name)
{
	material.setName(name);
}

std::string ShaderParams::getName()
{
	return material.getName();
}

GLuint ShaderParams::getTexture(int index)
{
	return textures[index];
}

bool ShaderParams::loadTexture(int index, QString filename)
{
	QImageReader reader(filename);
	reader.setAutoTransform(true);
	const QImage image = reader.read();
	if (!image.isNull()) {
		if (!material.getTexture().empty())
			removeTexture(index);
		float ratio = (float)image.width() / (float)image.height();
		material.setRatio(ratio);
		material.setTexture(filename.toStdString());
		textures[index] = loadTexture(image);
		return true;
	} else
		return false;
}

GLuint ShaderParams::loadTexture(QImage image)
{
	auto context = QOpenGLContext::currentContext();
	auto f = context->extraFunctions();

	GLuint name;
	GLsizei width = image.width();
	GLsizei height = image.height();

	f->glGenTextures(1, &name);
	f->glActiveTexture(GL_TEXTURE0);
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
	auto context = QOpenGLContext::currentContext();
	auto f = context->extraFunctions();

	material.setTexture("");
	if (textures[index] != 0 && textures[index] != defaultTextures[index])
		f->glDeleteTextures(1, &textures[index]);
	textures[index] = defaultTextures[index];
}

void ShaderParams::clearTextures()
{
	for (int i = 0; i < 4; i++)
		removeTexture(i);
}

pg::Material ShaderParams::getMaterial()
{
	return material;
}

void ShaderParams::setDefaultTexture(int index, GLuint name)
{
	defaultTextures[index] = name;
	if (textures[index] == 0)
		textures[index] = name;
}

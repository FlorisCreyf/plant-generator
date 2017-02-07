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

#include "shared_resources.h"
#include "graphics.h"
#include <QImage>

void SharedResources::create()
{
	createPrograms();
	createTextures();
}

void SharedResources::createPrograms()
{
	initializeOpenGLFunctions();

	GLuint vertModel = graphics::buildShader(GL_VERTEX_SHADER,
			"shaders/basic.vert");
	GLuint fragModel = graphics::buildShader(GL_FRAGMENT_SHADER,
			"shaders/basic.frag");
	GLuint vertFlat = graphics::buildShader(GL_VERTEX_SHADER,
			"shaders/flat.vert");
	GLuint fragFlat = graphics::buildShader(GL_FRAGMENT_SHADER,
			"shaders/flat.frag");
	GLuint vertWireframe = graphics::buildShader(GL_VERTEX_SHADER,
			"shaders/solid.vert");
	GLuint fragPoint = graphics::buildShader(GL_FRAGMENT_SHADER,
			"shaders/point.frag");
	GLuint vertLine = graphics::buildShader(GL_VERTEX_SHADER,
			"shaders/line.vert");
	GLuint geomLine = graphics::buildShader(GL_GEOMETRY_SHADER,
			"shaders/line.geom");
	GLuint fragLine = graphics::buildShader(GL_FRAGMENT_SHADER,
			"shaders/line.frag");

	{
		GLuint shaders[] = {vertModel, fragModel};
		programs[MODEL_SHADER] = graphics::buildProgram(shaders, 2);
	}
	{
		GLuint shaders[] = {vertFlat, fragFlat};
		programs[FLAT_SHADER] = graphics::buildProgram(shaders, 2);
	}
	{
		GLuint shaders[] = {vertWireframe, fragFlat};
		programs[WIREFRAME_SHADER] = graphics::buildProgram(shaders, 2);
	}
	{
		GLuint shaders[] = {vertFlat, fragPoint};
		programs[POINT_SHADER] = graphics::buildProgram(shaders, 2);
	}
	{
		GLuint shaders[] = {vertLine, geomLine, fragLine};
		programs[LINE_SHADER] = graphics::buildProgram(shaders, 3);
	}
}

void SharedResources::createTextures()
{
	QImage image("resources/dot.png");
	image.convertToFormat(QImage::Format_RGB32);
	GLsizei width = image.width();
	GLsizei height = image.height();

	glGenTextures(1, &textures[DOT_TEX]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[DOT_TEX]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_BGRA,
			GL_UNSIGNED_BYTE, image.bits());
}

GLuint SharedResources::getProgramName(Program program)
{
	return programs[program];
}

GLuint SharedResources::getTextureName(Texture texture)
{
	return textures[texture];
}

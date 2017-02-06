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

void SharedResources::create()
{
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
}

GLuint SharedResources::getProgramName(Program program)
{
	return programs[program];
}

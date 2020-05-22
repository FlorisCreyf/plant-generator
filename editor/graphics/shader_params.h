/* Plant Genererator
 * Copyright (C) 2019  Floris Creyf
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

#ifndef SHADER_PARAMS_H
#define SHADER_PARAMS_H

#include "plant_generator/material.h"
#include <QOpenGLFunctions_4_3_Core>

class ShaderParams {
	GLuint textures[4] = {0};
	GLuint defaultTextures[4] = {0};
	pg::Material material;

	GLuint loadTexture(QImage image);

public:
 	ShaderParams();
	ShaderParams(pg::Material material);
	long getID();
	void setName(std::string name);
	std::string getName();
	GLuint getTexture(int index);
	bool loadTexture(int index, QString filename);
	void removeTexture(int index);
	void clearTextures();
	pg::Material getMaterial();
	void setDefaultTexture(int index, GLuint name);
};

#endif /* SHADER_PARAMS_H */

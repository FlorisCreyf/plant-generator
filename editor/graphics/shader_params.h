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

#ifndef SHADER_PARAMS_H
#define SHADER_PARAMS_H

#include "plant_generator/material.h"
#include <QOpenGLExtraFunctions>

class ShaderParams {
	GLuint textures[pg::Material::MapQuantity] = {};
	GLuint defaultTextures[pg::Material::MapQuantity] = {};
	QString textureFiles[pg::Material::MapQuantity];
	pg::Material material;
	bool valid = false;

	GLuint loadTexture(QImage image);

public:
	ShaderParams();
	ShaderParams(pg::Material material);
	bool isValid() const;
	void initialize();
	void setName(std::string name);
	std::string getName();
	GLuint getTexture(int index);
	bool loadTexture(int index, QString filename);
	void removeTexture(int index);
	void clearTextures();
	void swapMaterial(pg::Material material);
	pg::Material getMaterial();
	void setDefaultTexture(int index, GLuint name);
};

#endif

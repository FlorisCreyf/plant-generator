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

#ifndef SHARED_RESOURCES_H
#define SHARED_RESOURCES_H

#include "shader_params.h"
#include "plant_generator/material.h"
#include <QOpenGLFunctions_4_3_Core>
#include <QOffscreenSurface>
#include <QObject>
#include <map>

class SharedResources : public QObject, protected QOpenGLFunctions_4_3_Core {
	Q_OBJECT

public:
	enum Shader {
		Solid, DynamicSolid,
		Wireframe, DynamicWireframe,
		Material, DynamicMaterial,
		Outline, DynamicOutline,
		Shadow, DynamicShadow,
		Point, Line, Flat,
		ShaderQuantity
	};
	enum Texture {
		DotTexture,
		BlackTexture,
		WhiteTexture,
		BlueTexture,
		TextureQuantity
	};

	SharedResources();
	void initialize();
	GLuint getShader(Shader shader);
	GLuint getTexture(Texture texture);
	GLuint addTexture(const QImage &image, GLenum baseformat,
		GLenum sizeformat);
	GLuint addDefaultTexture(const unsigned char color[3]);
	void removeTexture(GLuint texture);

	unsigned addMaterial(ShaderParams params);
	void updateMaterial(ShaderParams params, unsigned index);
	void removeMaterial(unsigned index);
	void clearMaterials();
	ShaderParams getMaterial(unsigned index);
	unsigned getMaterialCount() const;
	QOffscreenSurface *getSurface();

signals:
	void materialAdded(ShaderParams params);
	void materialModified(unsigned index);
	void materialRemoved(unsigned index);

private:
	GLuint programs[ShaderQuantity];
	GLuint textures[TextureQuantity];
	bool initialized;
	std::vector<ShaderParams> materials;
	QOffscreenSurface surface;

	void createPrograms();
	bool isCompiled(GLuint, const char *);
	bool openFile(const char *, std::string &);
	GLuint buildShader(GLenum, const char *, const char *);
	GLuint buildProgram(GLuint *, int);
};

#endif

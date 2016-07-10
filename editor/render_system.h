/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

#define GL_GLEXT_PROTOTYPES

#include "objects.h"
#include <vector>
#include <QOpenGLFunctions>

struct ShaderInfo {
	GLenum type;
	const char *filename;
};

struct GlobalUniforms {
	bt_mat4 vp;
	bt_vec3 cameraPosition;
};

class RenderSystem : protected QOpenGLFunctions
{
public:
	enum Type {
		MESH, LINE
	};

	RenderSystem();
	void init();
	void registerEntity(Mesh &m);
	void registerEntity(Line &l);
	void render(GlobalUniforms &gu);
	void loadShaders(ShaderInfo *info, int size);
	void setWireframe(int id, bool value, int s = 0, int e = 0);
	void updateVertices(int id, Type type, float *buffer, int size,
			bool resize);
	void updateTriangles(int id, Mesh *m, int size, bool resize);

private:
	float wireWidth;
	int wireProgram;
	struct Item {
		GLuint vao;
		int offset;
		int length;
		int program;
		union {
			float size;
			float width;
			bool wireframe;
		};
		int wireframeStart;
		int wireframeEnd;
		GLuint vb;
		GLuint eb;
	};
	std::vector<Item> mItems;
	std::vector<Item> lItems;
	std::vector<GLuint> programs;

	void setGlobalUniforms(GlobalUniforms &gu, GLuint program);
	void renderMesh(Item &item);
	void renderWire(Item &item);
	void renderLine(Item &item);
	void addVAO(Item &item, int attribs, int stride,
			const std::vector<float> &vertices);
	void switchProgram(int program, GlobalUniforms &gu);
};

#endif /* RENDER_SYSTEM_H */

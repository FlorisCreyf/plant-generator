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
	RenderSystem();
	void init();
	int load(GeometryComponent &g, int buffer = -1);
	void registerComponent(int buffer, RenderComponent r);
	void render(GlobalUniforms &gu, float color = 0.5f);
	void setVertexRange(int buffer, int index, int start, int end);
	void setTriangleRange(int buffer, int index, int start, int end);
	void setWireframe(int buffer, int index, int start, int end);
	void setPoints(int buffer, int index, int start, int end);
	void updateVertices(int buffer, float *v, int offset, int size);
	void updateTriangles(int buffer, unsigned short *v, int offset,
			int size);
	void setHidden(int buffer, int index, bool value);
	void loadShaders(ShaderInfo *info, int size);

private:
	int wireframeProgram;
	struct BufferObject {
		GLuint vao;
		GLuint vbo;
		GLuint ibo;
		std::vector<RenderComponent> r;
	};
	std::vector<BufferObject> buffers;
	std::vector<GLuint> programs;

	void loadTriangles(GeometryComponent &g, BufferObject &b);
	void loadVertices(GeometryComponent &g, BufferObject &b);
	void setGlobalUniforms(GlobalUniforms &gu, GLuint program);
	void switchProgram(int program, GlobalUniforms &gu);
	void renderWireframe(RenderComponent &r);
	void renderPoints(RenderComponent &r);
	void renderMesh(RenderComponent &r, GlobalUniforms &gu);
	void renderLines(RenderComponent &r, GlobalUniforms &gu);
};

#endif /* RENDER_SYSTEM_H */

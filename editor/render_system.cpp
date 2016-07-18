/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "render_system.h"
#include "terminal.h"
#include <cstdio>

RenderSystem::RenderSystem()
{
	wireframeProgram = 2;
}

void RenderSystem::init()
{
	initializeOpenGLFunctions();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0f, 1.0f);
}

void RenderSystem::loadTriangles(GeometryComponent &g, BufferObject &b)
{
	int size = g.triangles.size() * sizeof(unsigned short);
	GLenum usage = g.dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, b.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, &g.triangles[0], usage);
}

void RenderSystem::loadVertices(GeometryComponent &g, BufferObject &b)
{
	int size = g.vertices.size() * sizeof(float);
	GLenum usage = g.dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

	glBindBuffer(GL_ARRAY_BUFFER, b.vbo);
	glBufferData(GL_ARRAY_BUFFER, size, &g.vertices[0], usage);

	for (int i = 0; i < g.attribs; i++) {
		GLvoid *s = (void *)(sizeof(float)*3*i);
		glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, g.stride, s);
		glEnableVertexAttribArray(i);
	}
}

int RenderSystem::load(GeometryComponent &g, int buffer)
{
	if (buffer < 0) {
		BufferObject b;
		glGenVertexArrays(1, &b.vao);
		glGenBuffers(1, &b.vbo);
		if (g.triangles.size() > 0)
			glGenBuffers(1, &b.ibo);
		buffers.push_back(b);
		buffer = buffers.size() - 1;
	} else if (buffer < buffers.size()) {
		buffers[buffer].r.clear();
	} else
		return -1;

	glBindVertexArray(buffers[buffer].vao);
	loadVertices(g, buffers[buffer]);
	if (g.triangles.size() > 0)
		loadTriangles(g, buffers[buffer]);

	return buffer;
}

void RenderSystem::registerComponent(int buffer, RenderComponent r)
{
	if (buffer < buffers.size() && buffer >= 0)
		buffers[buffer].r.push_back(r);
}

void RenderSystem::setGlobalUniforms(GlobalUniforms &gu, GLuint program)
{
	GLint loc;
	bt_vec3 *pos = &gu.cameraPosition;

	loc = glGetUniformLocation(program, "vp");
	glUniformMatrix4fv(loc, 1, GL_FALSE, &gu.vp.m[0][0]);
	loc = glGetUniformLocation(program, "cameraPosition");
	glUniform4f(loc, pos->x, pos->y, pos->z, 0.0f);
}

void RenderSystem::switchProgram(int program, GlobalUniforms &gu)
{
	glUseProgram(programs[program]);
	setGlobalUniforms(gu, programs[program]);
}

void RenderSystem::setVertexRange(int buffer, int index, int start, int end)
{
	if (index >= 0 && buffer >= 0) {
		buffers[buffer].r[index].vertexRange[0] = start;
		buffers[buffer].r[index].vertexRange[1] = end;
	}
}

void RenderSystem::setTriangleRange(int buffer, int index, int start, int end)
{
	if (index >= 0 && buffer >= 0) {
		buffers[buffer].r[index].triangleRange[0] = start;
		buffers[buffer].r[index].triangleRange[1] = end;
	}
}

void RenderSystem::setWireframe(int buffer, int index, int start, int end)
{
	if (index >= 0 && buffer >= 0) {
		buffers[buffer].r[index].wireframeRange[0] = start;
		buffers[buffer].r[index].wireframeRange[1] = end;
	}
}

void RenderSystem::setPoints(int buffer, int index, int start, int end)
{
	if (index >= 0 && buffer >= 0) {
		buffers[buffer].r[index].pointRange[0] = start;
		buffers[buffer].r[index].pointRange[1] = end;
	}
}

void RenderSystem::renderWireframe(RenderComponent &r)
{
	int len = r.wireframeRange[1] - r.wireframeRange[0];
	void *offset = (void *)(r.wireframeRange[0] * sizeof(unsigned short));

	glPolygonOffset(-0.1f, -0.1f);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawElements(GL_TRIANGLES, len, GL_UNSIGNED_SHORT, offset);
	glPolygonOffset(0.0f, 0.0f);
	glDisable(GL_POLYGON_OFFSET_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void RenderSystem::renderPoints(RenderComponent &r)
{
	int len = r.pointRange[1] - r.pointRange[0];
	glPointSize(8);
	glDrawArrays(GL_POINTS, r.pointRange[0], len);
}

void RenderSystem::renderMesh(RenderComponent &r, GlobalUniforms &gu)
{
	int len = r.triangleRange[1] - r.triangleRange[0];
	void *offset = (void *)(r.triangleRange[0] * sizeof(unsigned short));
	switchProgram(r.program, gu);
	glDrawElements(GL_TRIANGLES, len, GL_UNSIGNED_SHORT, offset);

	if (r.wireframeRange[1] > 0) {
		switchProgram(wireframeProgram, gu);
		renderWireframe(r);
	}
}

void RenderSystem::renderLines(RenderComponent &r, GlobalUniforms &gu)
{
	int len = r.vertexRange[1] - r.vertexRange[0];

	switchProgram(r.program, gu);
	if (r.type == RenderComponent::LINES)
		glDrawArrays(GL_LINES, r.vertexRange[0], len);
	else
		glDrawArrays(GL_LINE_STRIP, r.vertexRange[0], len);

	if (r.pointRange[1] > 0) {
		switchProgram(wireframeProgram, gu);
		renderPoints(r);
	}
}

void RenderSystem::render(GlobalUniforms &gu, float color)
{
	glClearColor(color, color, color, 1.0);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (int i = 0; i < buffers.size(); i++) {
		glBindVertexArray(buffers[i].vao);
		for (int j = 0; j < buffers[i].r.size(); j++)
			if (buffers[i].r[j].type == RenderComponent::TRIANGLES)
				renderMesh(buffers[i].r[j], gu);
			else
				renderLines(buffers[i].r[j], gu);
	}

	glFlush();
}

void RenderSystem::updateVertices(int buffer, float *v, int offset, int size)
{
	offset *= sizeof(float);
	size *= sizeof(float);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[buffer].vbo);
	glBufferSubData(GL_ARRAY_BUFFER, offset, size, v);
}

void RenderSystem::updateTriangles(int buffer, unsigned short *v, int offset,
		int size)
{
	offset *= sizeof(unsigned short);
	size *= sizeof(unsigned short);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[buffer].ibo);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size, v);
}

void RenderSystem::loadShaders(ShaderInfo *r, int size)
{
	GLuint program = glCreateProgram();

	for (int i = 0; i < size; i++) {
		GLuint shader = glCreateShader(r[i].type);
		FILE *file = fopen(r[i].filename, "r");
		GLchar *buffer;
		int size;
		GLint status;

		if (file == NULL) {
			fclose(file);
			fprintf(stderr, "%s not found.\n", r[i].filename);
			continue;
		}

		fseek(file, 0, SEEK_END);
		size = ftell(file);
		rewind(file);

		buffer = new GLchar[size];
		if (!fread(buffer, 1, size, file)) {
			const char *f = r[i].filename;
			fprintf(stderr, "Failed to read %s.\n", f);
			delete[] buffer;
			continue;
		}

		fclose(file);

		glShaderSource(shader, 1, (const char**)&buffer, &size);
		glCompileShader(shader);

		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

		if (status == GL_FALSE) {
			GLsizei lsize;
			GLchar *log;

			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &lsize);
			log = new GLchar[lsize + 1];
			glGetShaderInfoLog(shader, lsize, &lsize, log);
			fprintf(stderr, BOLDWHITE "%s: "
					BOLDRED "error:"
					RESET "\n%s", r[i].filename, log);

			delete[] log;
		}

		glAttachShader(program, shader);
		delete[] buffer;
	}

	glLinkProgram(program);
	programs.push_back(program);
}

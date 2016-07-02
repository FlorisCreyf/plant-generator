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
	wireWidth = 1.0f;
	wireProgram = 2;
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

void RenderSystem::addVAO(Item &item, int attribs, int stride,
		const std::vector<float> &vertices)
{
	int vsize = vertices.size() * sizeof(float);
	glGenVertexArrays(1, &item.vao);
	glBindVertexArray(item.vao);
	glGenBuffers(1, &item.vb);
	glBindBuffer(GL_ARRAY_BUFFER, item.vb);
	glBufferData(GL_ARRAY_BUFFER, vsize, &vertices[0], GL_STATIC_DRAW);

	for (int i = 0; i < attribs; i++) {
		GLvoid *start = (void *)(sizeof(float) * 3 * i);
		glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, stride, start);
		glEnableVertexAttribArray(i);
	}
}

void RenderSystem::registerEntity(Mesh &m)
{
	int ts = m.triangles.size();
	unsigned short *t = &m.triangles[0];

	Item item = (Item){0, 0, m.tusage, m.program, 0};
	item.wireframe = false;
	addVAO(item, m.attribs, m.stride*2, m.vertices);

	glGenBuffers(1, &item.eb);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, item.eb);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ts, t, GL_DYNAMIC_DRAW);

	mItems.push_back(item);
}

void RenderSystem::registerEntity(Line &l)
{
	int vertices = l.vertices.size() / 3;
	Item item = (Item){0, 0, vertices, l.program, l.width};
	addVAO(item, l.attribs, l.stride*2, l.vertices);
	lItems.push_back(item);
}

void RenderSystem::setGlobalUniforms(GlobalUniforms &gu, GLuint program)
{
	GLint loc;
	bt_vec3 *v = &gu.cameraPosition;

	loc = glGetUniformLocation(program, "vp");
	glUniformMatrix4fv(loc, 1, GL_FALSE, &gu.vp.m[0][0]);
	loc = glGetUniformLocation(program, "cameraPosition");
	glUniform4f(loc, v->x, v->y, v->z, 0.0f);
}

void RenderSystem::switchProgram(int program, GlobalUniforms &gu)
{
	glUseProgram(programs[program]);
	setGlobalUniforms(gu, programs[program]);
}

void RenderSystem::setWireframe(int id, bool value)
{
	if (id >= 0)
		mItems[id].wireframe = value;
}

void RenderSystem::renderMesh(Item &item)
{
	glBindVertexArray(item.vao);
	glDrawElements(GL_TRIANGLES, item.length, GL_UNSIGNED_SHORT, NULL);
}

void RenderSystem::renderWire(Item &item)
{
	glPolygonOffset(-0.1f, -0.1f);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glLineWidth(wireWidth);
	renderMesh(item);

	glPolygonOffset(0.0f, 0.0f);
	glDisable(GL_POLYGON_OFFSET_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void RenderSystem::renderLine(Item &item)
{
	glBindVertexArray(item.vao);
	glLineWidth(item.width);
	glDrawArrays(GL_LINES, item.offset, item.length);
}

void RenderSystem::render(GlobalUniforms &gu)
{
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (int i = 0; i < (int)mItems.size(); i++) {
		switchProgram(mItems[i].program, gu);
		renderMesh(mItems[i]);

		if (mItems[i].wireframe) {
			switchProgram(wireProgram, gu);
			renderWire(mItems[i]);
		}
	}

	for (int i = 0; i < (int)lItems.size(); i++) {
		switchProgram(lItems[i].program, gu);
		glLineWidth(10.0f);
		renderLine(lItems[i]);
	}

	glFlush();
}

void RenderSystem::updateVertices(int id, float *buffer, int size)
{
	Item item = mItems[id];
	glBindBuffer(GL_ARRAY_BUFFER, item.vb);
	glBufferSubData(GL_ARRAY_BUFFER, 0, size*sizeof(float), buffer);
}

void RenderSystem::updateTriangles(int id, unsigned short *buffer, int size)
{
	int s = size * sizeof(unsigned short);
	Item *item = &mItems[id];
	item->length = size;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, item->eb);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, s, buffer);
}

void RenderSystem::loadShaders(ShaderInfo *info, int size)
{
	GLuint program = glCreateProgram();

	for (int i = 0; i < size; i++) {
		GLuint shader = glCreateShader(info[i].type);
		FILE *file = fopen(info[i].filename, "r");
		GLchar *buffer;
		int size;
		GLint status;

		if (file == NULL) {
			fclose(file);
			fprintf(stderr, "%s not found.\n", info[i].filename);
			continue;
		}

		fseek(file, 0, SEEK_END);
		size = ftell(file);
		rewind(file);

		buffer = new GLchar[size];
		if (!fread(buffer, 1, size, file)) {
			const char *f = info[i].filename;
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
					RESET "\n%s", info[i].filename, log);

			delete[] log;
		}

		glAttachShader(program, shader);
		delete[] buffer;
	}

	glLinkProgram(program);
	programs.push_back(program);
}

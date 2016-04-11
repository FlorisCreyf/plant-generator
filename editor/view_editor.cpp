/* 
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "view_editor.h"
#include "terminal.h"
#include <cstdio>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtOpenGL/QGLFormat>

ViewEditor::ViewEditor(QWidget *parent) : QGLWidget(parent), grid(10)
{
	QGLFormat format;
	format.setVersion(3, 3);
	format.setProfile(QGLFormat::CoreProfile);
	format.setSampleBuffers(true);
	setFormat(format);
	makeCurrent();
}

ViewEditor::~ViewEditor()
{

}

void ViewEditor::initializeGL()
{
	glClearColor(0.5, 0.5, 0.5, 1.0);

	glGenVertexArrays(2, VAOs);
	initializeGrid();
	initializeTree();

	ShaderInfo shaders[] = {
		{GL_VERTEX_SHADER, "shaders/basic.vert"},
		{GL_FRAGMENT_SHADER, "shaders/basic.frag"},
		{GL_VERTEX_SHADER, "shaders/flat.vert"},
		{GL_FRAGMENT_SHADER, "shaders/flat.vert"}
	};

	programs[0] = loadShaders(&shaders[0], 2);
	programs[1] = loadShaders(&shaders[2], 2);
}

void ViewEditor::resizeGL(int width, int height)
{
	float aspectRatio = (float)width / (float)height;
	glViewport(0, 0, width, height);
	glDraw();
}

void ViewEditor::initializeGrid()
{

}

void ViewEditor::initializeTree()
{

}

void ViewEditor::keyPressEvent(QKeyEvent *event)
{

}

void ViewEditor::mousePressEvent(QMouseEvent *event)
{

}

void ViewEditor::mouseMoveEvent(QMouseEvent *event)
{

}

void ViewEditor::paintGL()
{

}

GLuint ViewEditor::loadShaders(ShaderInfo *info, int size)
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
		fread(buffer, 1, size, file);
		fclose(file);

		glShaderSource(shader, 1, &buffer, &size);
		glCompileShader(shader);

		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
		
		if (status == GL_FALSE) {
			GLsizei lsize;
			GLchar *log;
			
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &lsize);
			log = new GLchar[lsize + 1];
			glGetShaderInfoLog(shader, lsize, &lsize, log);
			fprintf(stderr, BOLDWHITE "%s: " BOLDRED "error:" RESET
					"\n%s", info[i].filename, log);

			delete[] log;
		}
		
		glAttachShader(program, shader);
		delete[] buffer;
	}

	glLinkProgram(program);
	return program;
}


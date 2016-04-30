/* 
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "view_editor.h"
#include "grid.h"
#include "bluetree.h"
#include "terminal.h"
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtOpenGL/QGLFormat>

ViewEditor::ViewEditor(QWidget *parent) : QGLWidget(parent), grid(Grid(5))
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
	bt_delete_tree(tree);
	delete[] vbo;
	delete[] ebo;
}

void ViewEditor::initializeGL()
{
	glClearColor(0.5, 0.5, 0.5, 1.0);

	glGenVertexArrays(2, VAOs);
	initializeGrid();
	initializeTree();

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0f, 1.0f);

	ShaderInfo shaders[] = {
		{GL_VERTEX_SHADER, "shaders/flat.vert"},
		{GL_FRAGMENT_SHADER, "shaders/flat.frag"},
		{GL_VERTEX_SHADER, "shaders/basic.vert"},
		{GL_FRAGMENT_SHADER, "shaders/basic.frag"}
	};

	programs[0] = loadShaders(&shaders[0], 2);
	programs[1] = loadShaders(&shaders[2], 2);
}

void ViewEditor::resizeGL(int width, int height)
{
	float aspectRatio = (float)width / (float)height;
	camera.setPerspective(45.0f, 0.5f, 200.0f, aspectRatio);

	glViewport(0, 0, width, height);
	glDraw();
}

void ViewEditor::initializeGrid()
{
	GLfloat *vertices = grid.getBuffer();
	int size = grid.getVertexCount() * 3 * sizeof(float);
	GLuint buffer;

	glBindVertexArray(VAOs[0]);
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glEnableVertexAttribArray(0);
}

void ViewEditor::initializeTree()
{
	int ebSize = 1200;
	int vbSize = 1200 * 2;
	GLsizei stride = sizeof(float) * 3;
	GLuint buffer;
	
	vbo = new GLfloat[vbSize];
	ebo = new GLushort[ebSize];	

	tree = bt_new_tree();
	bt_set_trunk_radius(tree, 0.25f);
	bt_set_resolution(tree, 8);
	bt_set_max_branch_depth(tree, 1);
	bt_generate_structure(tree);
	bt_generate_mesh(tree, vbo, vbSize, ebo, ebSize);

	glBindVertexArray(VAOs[1]);
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, vbSize, vbo, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride*2, (GLvoid *)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride*2, 
			(GLvoid *)(sizeof(float) * 3));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ebSize, ebo, GL_STATIC_DRAW);
}

void ViewEditor::keyPressEvent(QKeyEvent *event)
{

}

void ViewEditor::mousePressEvent(QMouseEvent *event)
{
	QPoint point = event->pos();
	camera.setStartCoordinates(point.x(), point.y());

	setFocus();
}

void ViewEditor::mouseMoveEvent(QMouseEvent *event)
{
	QPoint point = event->pos();
	camera.setCoordinates(point.x(), point.y());
	
	glDraw();
}

void ViewEditor::initializeShader(bt_mat4 *mvp)
{
	bt_vec3 c = camera.getPosition();	
	GLint loc;

	loc = glGetUniformLocation(programs[1], "matrix");
	glUniformMatrix4fv(loc, 1, GL_FALSE, &(mvp->m[0][0]));
	
	loc = glGetUniformLocation(programs[1], "cameraPos");
	glUniform4f(loc, c.x, c.y, c.z, 0.0f);
}

void ViewEditor::paintGL()
{
	bt_mat4 mvp = camera.getCrystalBallMatrix();
	GLint mLocation;

	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(programs[1]);
	initializeShader(&mvp);		
	
	glBindVertexArray(VAOs[1]);
	
	glDrawElements(GL_TRIANGLES, bt_get_ebo_size(tree),
			GL_UNSIGNED_SHORT, NULL);

	glUseProgram(programs[0]);
	mLocation = glGetUniformLocation(programs[1], "matrix");
	glUniformMatrix4fv(mLocation, 1, GL_FALSE, &(mvp.m[0][0]));

	glPolygonOffset(-1.0f, -1.0f);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	glLineWidth(1);
	glDrawElements(GL_TRIANGLES, bt_get_ebo_size(tree),
			GL_UNSIGNED_SHORT, NULL);
	
	glDisable(GL_POLYGON_OFFSET_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glBindVertexArray(VAOs[0]);
	glLineWidth(2);
	glDrawArrays(GL_LINES, 0, 4);
	glLineWidth(0.5);
	glDrawArrays(GL_LINES, 4, grid.getVertexCount() - 4);

	glFlush();
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
		if (!fread(buffer, 1, size, file)) {
			fprintf(stderr, "Failed to read %s.\n",
					info[i].filename);
			continue;
		}

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


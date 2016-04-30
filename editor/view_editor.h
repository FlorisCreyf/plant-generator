/* 
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef VIEW_EDITOR_H
#define VIEW_EDITOR_H

#define GL_GLEXT_PROTOTYPES

#include "bluetree.h"
#include "grid.h"
#include "camera.h"
#include <GL/gl.h>
#include <QtOpenGL/QGLWidget>

class ViewEditor : public QGLWidget
{
	Q_OBJECT

public:
	ViewEditor(QWidget *parent = 0);
	~ViewEditor();
	
protected:
	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);
	void mousePressEvent(QMouseEvent *);
	void keyPressEvent(QKeyEvent *);
	void mouseMoveEvent(QMouseEvent *);

private:
	bt_tree tree;
	Camera camera;
	Grid grid;

	GLint programs[2];
	struct ShaderInfo {
		GLenum type;
		const char *filename;
	};
	
	GLuint VAOs[2];
	GLfloat *vbo;
	GLushort *ebo;

	void initializeGrid();
	void initializeTree();
	void initializeShader(bt_mat4 *mvp);
	GLuint loadShaders(ShaderInfo *info, int size);
};

#endif /* VIEW_EDITOR_H */


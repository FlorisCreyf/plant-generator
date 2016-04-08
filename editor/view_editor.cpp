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
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtOpenGL/QGLFormat>

ViewEditor::ViewEditor(QWidget *parent) : QGLWidget(parent)
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
}

void ViewEditor::resizeGL(int w, int h)
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

}


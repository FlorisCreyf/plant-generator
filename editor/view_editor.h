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
#include "render_system.h"
#include "scene.h"
#include "camera.h"
#include <QtGui/QOpenGLFunctions>

#include <QOpenGLWidget>


class ViewEditor : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
	ViewEditor(QWidget *parent = 0);
	~ViewEditor();

	void exportObject(const char *filename);

protected:
	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);
	void mousePressEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void keyPressEvent(QKeyEvent *);
	void keyReleaseEvent(QKeyEvent *);
	void mouseMoveEvent(QMouseEvent *);

private:
	bool ctrl;
	bool shift;
	bool midButton;

	bt_tree tree;
	Scene scene;
	Camera camera;
	QOpenGLContext ctx;
	RenderSystem rs;

	void initializeTree();
};

#endif /* VIEW_EDITOR_H */

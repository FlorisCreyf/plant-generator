/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef CURVE_EDITOR_H
#define CURVE_EDITOR_H

#include "render_system.h"
#include "vector.h"
#include <QtGui/QOpenGLFunctions>
#include <QOpenGLWidget>

class CurveEditor : public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT

public:
	CurveEditor(QWidget *parent = 0);

protected slots:
	void onFloat(bool);

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
	RenderSystem rs;
	bool init;
	int height;
	int width;
	std::vector<bt_vec3> controls;
	int selected;

	void createInterface();
};

#endif /* CURVE_EDITOR_H */

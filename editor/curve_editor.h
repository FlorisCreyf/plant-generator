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

public slots:
	void setControls(std::vector<bt_vec3> controls);
	void setEnabled(bool enabled);

signals:
	void controlsChanged(std::vector<bt_vec3> controls);

protected slots:
	void onFloat(bool);

protected:
	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);
	void mousePressEvent(QMouseEvent *);
	void mouseDoubleClickEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void keyPressEvent(QKeyEvent *);
	void keyReleaseEvent(QKeyEvent *);
	void mouseMoveEvent(QMouseEvent *);

private:
	RenderSystem rs;
	Entity ui;
	bool enabled;
	int height;
	int width;
	std::vector<bt_vec3> controls;
	bt_vec3 curve[4];
	int curveIndex;
	int selected;

	void createInterface();
	int createBackground(int);
	int createControlLines(int);
	int createCurve(int);
	void updateCurve();
	void drawCurve();
	void insertCurve(int, float, float);
	bool reinsertCurve(float);
	bool omitCurve(float);
	void placeOuterControl(float, float);
	void placeInnerControl(float, float);
	void placeTerminalControl(bool, float);
};

#endif /* CURVE_EDITOR_H */

/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef CURVE_BUTTON_H
#define CURVE_BUTTON_H

#include "render_system.h"
#include <QtGui/QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QtWidgets>

class CurveButton : public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT

public:
	CurveButton(QWidget *parent = 0);
	void setControls(std::vector<bt_vec3> controls);

protected:
	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);
	void mousePressEvent(QMouseEvent *);

private:
	void createGeometry(GeometryComponent &g);

	RenderSystem rs;
	std::vector<bt_vec3> controls;
};

class CurveButtonWidget : public QWidget
{
	Q_OBJECT

public:
	CurveButtonWidget(QWidget *parent = 0);
	void setControls(std::vector<bt_vec3> controls);

protected:
	void paintEvent(QPaintEvent *);
	QSize sizeHint() const;

private:
	CurveButton *cb;
};

#endif /* CURVE_BUTTON_H */

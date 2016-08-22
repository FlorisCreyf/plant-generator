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
	void setControls(tm_vec3 *controls, int size);
	void setControls(vector<tm_vec3> controls);
	vector<tm_vec3> getControls();

protected:
	void initializeGL();
	void paintGL();

private:
	void createGeometry();

	int buffer;
	RenderSystem rs;
	Entity curve;
	vector<tm_vec3> controls;
};

class CurveButtonWidget : public QWidget
{
	Q_OBJECT

public:
	CurveButtonWidget(QString name, QWidget *parent = 0);
	CurveButton *getCurveButton();
	QString getName();
	void select();

signals:
	void selected(CurveButtonWidget *);

protected:
	QSize sizeHint() const;
	void paintEvent(QPaintEvent *);
	void mousePressEvent(QMouseEvent *);

private:
	CurveButton *button;
	QString name;
};

#endif /* CURVE_BUTTON_H */

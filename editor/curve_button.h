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
	void setControls(std::vector<tm_vec3> controls);
	std::vector<tm_vec3> getControls();

protected:
	void initializeGL();
	void paintGL();

private:
	void createGeometry();

	int buffer;
	RenderSystem rs;
	Entity curve;
	std::vector<tm_vec3> controls;
};

class CurveButtonWidget : public QWidget
{
	Q_OBJECT

public:
	CurveButtonWidget(QString name, QWidget *parent = 0);
	void setControls(std::vector<tm_vec3> controls);
	void select();

signals:
	void selected(std::vector<tm_vec3>, QString);

protected:
	void paintEvent(QPaintEvent *);
	QSize sizeHint() const;
	void mousePressEvent(QMouseEvent *);

private:
	CurveButton *cb;
	QString name;
};

#endif /* CURVE_BUTTON_H */

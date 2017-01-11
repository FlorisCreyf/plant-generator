/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "curve_button.h"
#include "primitives.h"

CurveButton::CurveButton(QWidget *parent) : QOpenGLWidget(parent)
{
	curve.renderComponent.resize(1);
	curve.renderComponent[0].type = RenderComponent::LINE_STRIP;
}

void CurveButton::setControls(TMvec3 *controls, int size)
{
	vector<TMvec3> v;
	v.insert(v.begin(), controls, controls+size);
	setControls(v);
}

void CurveButton::setControls(vector<TMvec3> controls)
{
	this->controls = controls;
	createGeometry();

	if (buffer >= 0) {
		rs.load(curve.geometry, buffer);
		rs.registerComponent(buffer, curve.renderComponent[0]);
		update();
	}
}

vector<TMvec3> CurveButton::getControls()
{
	return controls;
}

void CurveButton::initializeGL()
{
	initializeOpenGLFunctions();
	rs.init();

	ShaderInfo shaders[] = {
		{GL_VERTEX_SHADER, "shaders/flat.vert"},
		{GL_FRAGMENT_SHADER, "shaders/flat.frag"},
	};

	rs.loadShaders(&shaders[0], 2);

	if (curve.geometry.vertices.size() == 0) {
		createGeometry();
		buffer = rs.load(curve.geometry);
	} else {
		buffer = rs.load(curve.geometry);
		rs.registerComponent(buffer, curve.renderComponent[0]);
		update();
	}
}

void CurveButton::createGeometry()
{
	GeometryComponent g;
	createPath(g, controls, 10, (TMvec3){.6f, 0.6f, 0.6f});
	curve.renderComponent[0].vertexRange[1] = g.vertices.size() / 6;
	curve.geometry = g;
}

void CurveButton::paintGL()
{
	GlobalUniforms gu;
	gu.vp = (TMmat4){
			1.8f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.9f, 0.0f,
			0.0f, 1.8f, 0.0f, 0.0f,
			-0.9f, -0.9f, 0.0f, 1.0f};
	rs.render(gu, 0.32);
}

CurveButtonWidget::CurveButtonWidget(QString name, QWidget *parent) :
		QWidget(parent)
{
	QHBoxLayout *layout = new QHBoxLayout(this);
	button = new CurveButton(this);
	layout->addWidget(button);
	layout->setMargin(1.5);
	setCursor(Qt::PointingHandCursor);
	setLayout(layout);
	this->name = name;
}

QSize CurveButtonWidget::sizeHint() const
{
	return QSize(26, 19);
}

CurveButton *CurveButtonWidget::getCurveButton()
{
	return button;
}

QString CurveButtonWidget::getName()
{
	return name;
}

void CurveButtonWidget::select()
{
	emit selected(this);
}

void CurveButtonWidget::mousePressEvent(QMouseEvent *)
{
	select();
}

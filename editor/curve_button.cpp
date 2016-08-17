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

CurveButton::CurveButton(QWidget *parent)
{
	curve.renderComponent.resize(1);
	curve.renderComponent[0].type = RenderComponent::LINE_STRIP;
}

void CurveButton::setControls(std::vector<tm_vec3> controls)
{
	this->controls = controls;
	createGeometry();

	if (buffer >= 0) {
		rs.load(curve.geometry, buffer);
		rs.registerComponent(buffer, curve.renderComponent[0]);
		update();
	}
}

std::vector<tm_vec3> CurveButton::getControls()
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
	createPath(g, controls, 10, (tm_vec3){.2f, 0.46f, 0.6f});
	curve.geometry = g;
	curve.renderComponent[0].vertexRange[1] = g.vertices.size() / 6;
}

void CurveButton::paintGL()
{
	GlobalUniforms gu;
	gu.vp = (tm_mat4){
			1.8f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.9f, 0.0f,
			0.0f, 1.8f, 0.0f, 0.0f,
			-0.9f, -0.9f, 0.0f, 1.0f};
	rs.render(gu, 0.3f);
}

CurveButtonWidget::CurveButtonWidget(QString name, QWidget *parent)
{
	QHBoxLayout *b = new QHBoxLayout(this);
	cb = new CurveButton(this);
	b->addWidget(cb);
	b->setMargin(1.5);
	setCursor(Qt::PointingHandCursor);
	setStyleSheet("border:2px solid #999;");
	setLayout(b);

	this->name = name;
}

void CurveButtonWidget::paintEvent(QPaintEvent *e)
{
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

QSize CurveButtonWidget::sizeHint() const
{
	return QSize(26, 16);
}

void CurveButtonWidget::setControls(std::vector<tm_vec3> controls)
{
	cb->setControls(controls);
}

void CurveButtonWidget::select()
{
	emit selected(cb->getControls(), name);
}

void CurveButtonWidget::mousePressEvent(QMouseEvent *)
{
	emit selected(cb->getControls(), name);
}

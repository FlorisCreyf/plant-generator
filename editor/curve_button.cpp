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

}

void CurveButton::setControls(std::vector<bt_vec3> controls)
{
	GeometryComponent g = {};
	RenderComponent curve = {};

	this->controls = controls;
	createGeometry(g);
	curve.type = RenderComponent::LINE_STRIP;
	curve.vertexRange[1] = g.vertices.size() / 6;

	rs.load(g, 0);
	rs.registerComponent(0, curve);
	update();
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

	GeometryComponent g;
	createGeometry(g);
	rs.load(g);
}

void CurveButton::createGeometry(GeometryComponent &g)
{
	createPath(g, controls, 10, (bt_vec3){.2f, 0.46f, 0.6f});
}

void CurveButton::paintGL()
{
	GlobalUniforms gu;
	gu.vp = (bt_mat4){
			1.8f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.9f, 0.0f,
			0.0f, 1.8f, 0.0f, 0.0f,
			-0.9f, -0.9f, 0.0f, 1.0f};
	rs.render(gu, 0.3f);
}

void CurveButton::resizeGL(int width, int height)
{

}

void CurveButton::mousePressEvent(QMouseEvent *)
{

}

CurveButtonWidget::CurveButtonWidget(QWidget *parent)
{
	QHBoxLayout *b = new QHBoxLayout(this);
	cb = new CurveButton(this);
	b->addWidget(cb);
	b->setMargin(1.5);
	setStyleSheet("border:2px solid #999;");
	setLayout(b);
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

void CurveButtonWidget::setControls(std::vector<bt_vec3> controls)
{
	cb->setControls(controls);
}

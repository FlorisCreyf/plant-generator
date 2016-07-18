/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "curve_editor.h"
#include "primitives.h"
#include "curve.h"
#include <QtGui/QMouseEvent>
#include <QtGui/QTabBar>
#include <math.h>

CurveEditor::CurveEditor(QWidget *parent) : QOpenGLWidget(parent)
{
	init = true;
	selected = -1;
}

void CurveEditor::onFloat(bool topLevel)
{
	QList<QTabBar *> l = parent()->parent()->findChildren<QTabBar*>();
	if (l.size() > 0)
		l[0]->setDrawBase(false);
}

void CurveEditor::createInterface()
{
	GeometryComponent g;
	RenderComponent plane = {};
	RenderComponent grid = {};
	RenderComponent curve = {};
	int size;
	int lineStart;

	createBezier(g, controls, 100);
	lineStart = g.vertices.size() / 6;
	createLine(g, controls);
	curve.type = RenderComponent::LINE_STRIP;
	curve.vertexRange[1] = size = g.vertices.size() / 6;
	curve.pointRange[0] = lineStart;
	curve.pointRange[1] = g.vertices.size() / 6;

	createGrid(g, 3, 1.0f/3.0f);
	grid.type = RenderComponent::LINES;
	grid.vertexRange[0] = size;
	grid.vertexRange[1] = size = g.vertices.size() / 6;

	createPlane(g, (bt_vec3){1, 0, 0}, (bt_vec3){0, 0, 1});
	plane.type = RenderComponent::TRIANGLES;
	plane.vertexRange[0] = size;
	plane.vertexRange[1] = size = g.vertices.size() / 6;
	plane.triangleRange[1] = g.triangles.size();

	int buffer = rs.load(g);
	rs.registerComponent(buffer, plane);
	rs.registerComponent(buffer, curve);
	rs.registerComponent(buffer, grid);
}

void CurveEditor::initializeGL()
{
	initializeOpenGLFunctions();
	rs.init();
	onFloat(false);
	connect(parent(), SIGNAL(topLevelChanged(bool)), this,
			SLOT(onFloat(bool)));

	controls.push_back((bt_vec3){-1, -.1, 1});
	controls.push_back((bt_vec3){0, -.1, -0.5f});
	controls.push_back((bt_vec3){0, -.1, 0.5f});
	controls.push_back((bt_vec3){1, -.1, -1});

	ShaderInfo shaders[] = {
		{GL_VERTEX_SHADER, "shaders/flat.vert"},
		{GL_FRAGMENT_SHADER, "shaders/flat.frag"},
	};
	rs.loadShaders(&shaders[0], 2);

	createInterface();
}

void CurveEditor::resizeGL(int width, int height)
{
	glViewport(0, 0, width, height);
	this->width = width;
	this->height = height;
}

void CurveEditor::keyPressEvent(QKeyEvent *event)
{

}

void CurveEditor::keyReleaseEvent(QKeyEvent *event)
{

}

void CurveEditor::mousePressEvent(QMouseEvent *event)
{
	QPoint p = event->pos();
	for (int i = 0; i < controls.size(); i++) {
		int cx = (controls[i].x*0.9f + 1.0f) * width*0.5f;
		int cy = height - (controls[i].z*0.9f + 1.0f) * height*0.5f;
		if (sqrt(pow(p.x() - cx, 2) + pow(p.y() - cy, 2)) < 8)
			selected = i;
	}
}

void CurveEditor::mouseReleaseEvent(QMouseEvent *event)
{
	selected = -1;
}

void CurveEditor::mouseMoveEvent(QMouseEvent *event)
{
	if (selected < 0)
		return;

	QPoint p = event->pos();
	float x = (2.0f*p.x()/(width - 1) - 1.0f) * (10.f/9.0f);
	float y = (1.0f - 2.0f*p.y()/(height - 1)) * (10.f/9.0f);
	controls[selected].x = x;
	controls[selected].z = y;

	GeometryComponent g;
	createBezier(g, controls, 100);
	createLine(g, controls);
	rs.updateVertices(0, &g.vertices[0], 0, g.vertices.size());

	update();
}

void CurveEditor::paintGL()
{
	GlobalUniforms gu;
	gu.vp = (mat4){
			0.9f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.9f, 0.0f,
			0.0f, 0.9f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f};
	rs.render(gu, 0.3f);
}

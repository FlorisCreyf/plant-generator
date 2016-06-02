/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "view_editor.h"
#include "file_exporter.h"
#include "primitives.h"
#include "collision.h"
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtOpenGL/QGLFormat>

ViewEditor::ViewEditor(QWidget *parent) : QGLWidget(parent)
{
	QGLFormat format;
	format.setVersion(3, 3);
	format.setProfile(QGLFormat::CoreProfile);
	format.setSampleBuffers(true);
	setFormat(format);
	makeCurrent();

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0f, 1.0f);
}

ViewEditor::~ViewEditor()
{
	bt_delete_tree(tree);
}

void ViewEditor::exportObject(const char *filename)
{
	FileExporter e;
	Mesh m = scene.getMesh(0);
	e.setVertices(&m.vertices[0], bt_get_vbo_size(tree));
	e.setTriangles(&m.triangles[0], bt_get_ebo_size(tree));
	e.exportObj(filename);
}

void ViewEditor::initializeGL()
{
	Line l = createGrid(5);
	rs.registerEntity(l);
	initializeTree();

	ShaderInfo shaders[] = {
		{GL_VERTEX_SHADER, "shaders/flat.vert"},
		{GL_FRAGMENT_SHADER, "shaders/flat.frag"},
		{GL_VERTEX_SHADER, "shaders/basic.vert"},
		{GL_FRAGMENT_SHADER, "shaders/basic.frag"},
		{GL_VERTEX_SHADER, "shaders/solid.vert"},
		{GL_FRAGMENT_SHADER, "shaders/flat.frag"}
	};

	rs.loadShaders(&shaders[0], 2);
	rs.loadShaders(&shaders[2], 2);
	rs.loadShaders(&shaders[4], 2);
}

void ViewEditor::resizeGL(int width, int height)
{
	float aspectRatio = (float)width / (float)height;
	camera.setWindowSize(width, height);
	camera.setPerspective(45.0f, 1.0f, 100.0f, aspectRatio);
	glViewport(0, 0, width, height);
	glDraw();
}

void ViewEditor::initializeTree()
{
	const int eSize = 4000;
	const int vSize = 4000 * 2;

	Mesh m;
	m.attribs = 2;
	m.stride = sizeof(float) * 3;
	m.vertices.resize(vSize);
	m.triangles.resize(eSize);
	m.program = 1;

	tree = bt_new_tree();
	bt_set_trunk_radius(tree, 0.25f);
	bt_set_resolution(tree, 12);
	bt_set_max_branch_depth(tree, 1);
	bt_generate_structure(tree);
	bt_generate_mesh(tree, &m.vertices[0], vSize, &m.triangles[0], eSize);

	m.tusage = bt_get_ebo_size(tree);
	m.vusage = bt_get_vbo_size(tree);
	scene.add(m);
	rs.registerEntity(m);

	bt_aabb b = bt_create_aabb(&m.vertices[0], m.vusage*12);
	Line box = createBox(b);
	rs.registerEntity(box);
}

void ViewEditor::keyPressEvent(QKeyEvent *event)
{

}

void ViewEditor::mousePressEvent(QMouseEvent *event)
{
	QPoint p = event->pos();
	camera.setStartCoordinates(p.x(), p.y());
	setFocus();

	int s = scene.getSelected();
	rs.setWireframe(s, false);
	s = scene.setSelected(camera, p.x(), p.y());
	rs.setWireframe(s, true);

	glDraw();
}

void ViewEditor::mouseMoveEvent(QMouseEvent *event)
{
	QPoint point = event->pos();
	camera.setCoordinates(point.x(), point.y());
	glDraw();
}

void ViewEditor::paintGL()
{
	GlobalUniforms gu;

	gu.vp = camera.getVP();
	gu.cameraPosition = camera.getPosition();
	rs.render(gu);
}

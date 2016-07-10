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

ViewEditor::ViewEditor(QWidget *parent) : QOpenGLWidget(parent)
{
	ctrl = shift = midButton = false;
	camera.action = Camera::NONE;
	setFocus();
}

ViewEditor::~ViewEditor()
{
	bt_delete_tree(tree);
}

void ViewEditor::exportObject(const char *filename)
{
	FileExporter e;
	Mesh *m = scene.getMesh(0);
	e.setVertices(&m->vertices[0], bt_get_vbo_size(tree));
	e.setTriangles(&m->triangles[0], bt_get_ebo_size(tree));
	e.exportObj(filename);
}

void ViewEditor::initializeGL()
{
	initializeOpenGLFunctions();
	rs.init();

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
	camera.setPerspective(45.0f, 0.1f, 100.0f, aspectRatio);
	glViewport(0, 0, width, height);
	paintGL();
}

void ViewEditor::initializeTree()
{
	const int eSize = 4000;
	const int vSize = 4000;

	Mesh m;
	m.attribs = 2;
	m.stride = sizeof(float) * 3;
	m.vertices.resize(vSize);
	m.triangles.resize(eSize);
	m.program = 1;

	tree = bt_new_tree();
	bt_set_trunk_radius(tree, 0, 0.2f);
	bt_set_resolution(tree, 0, 12);
	bt_set_max_branch_depth(tree, 1);
	bt_generate_structure(tree);
	bt_generate_mesh(tree, &m.vertices[0], vSize, &m.triangles[0], eSize);

	m.tusage = bt_get_ebo_size(tree);
	m.vusage = bt_get_vbo_size(tree);
	scene.add(m);
	rs.registerEntity(m);

	emit selectionChanged(tree, 0);
}

void ViewEditor::keyPressEvent(QKeyEvent *event)
{
	switch (event->key()) {
	case Qt::Key_Control:
		ctrl = true;
		break;
	case Qt::Key_Shift:
		shift = true;
		break;
	}
}

void ViewEditor::keyReleaseEvent(QKeyEvent *event)
{
	switch (event->key()) {
	case Qt::Key_Control:
		ctrl = false;
		break;
	case Qt::Key_Shift:
		shift = false;
		break;
	}
}

void ViewEditor::selectBranch(int x, int y)
{
	int sel;
	int branch;
	int s;
	int e;

	sel = scene.getSelected();
	rs.setWireframe(sel, false);
	sel = scene.setSelected(camera, x, y);
	branch = scene.setSelectedBranch(camera, x, y, tree);
	if (sel >= 0 && branch >= 0) {
		s = bt_get_ebo_start(tree, branch);
		e = bt_get_ebo_end(tree, branch);
		rs.setWireframe(sel, true, s, e);

		emit selectionChanged(tree, branch);
	}
}

void ViewEditor::mousePressEvent(QMouseEvent *event)
{
	QPoint p = event->pos();
	midButton = false;

	if (event->button() == Qt::MidButton) {
		camera.setStartCoordinates(p.x(), p.y());
		midButton = true;

		if (ctrl && !shift)
			camera.action = Camera::ZOOM;
		else if (shift && !ctrl)
			camera.action = Camera::PAN;
		else
			camera.action = Camera::ROTATE;
	} else if (event->button() == Qt::RightButton) {
		selectBranch(p.x(), p.y());
		update();
	}

	setFocus();
}

void ViewEditor::mouseReleaseEvent(QMouseEvent *event)
{
	camera.action = Camera::NONE;
}

void ViewEditor::mouseMoveEvent(QMouseEvent *event)
{
	QPoint point = event->pos();

	switch (camera.action) {
	case Camera::ZOOM:
		camera.zoom(point.x(), point.y());
		break;
	case Camera::ROTATE:
		camera.setCoordinates(point.x(), point.y());
		break;
	case Camera::PAN:
		camera.setPan(point.x(), point.y());
		break;
	}

	update();
}

void ViewEditor::paintGL()
{
	GlobalUniforms gu;
	gu.vp = camera.getVP();
	gu.cameraPosition = camera.getPosition();
	rs.render(gu);
}

void ViewEditor::updateWireframe()
{
	int sel = scene.getSelected();
	if (sel >= 0) {
		int branch = scene.getSelectedBranch();
		int s1 = bt_get_ebo_start(tree, branch);
		int e1 = bt_get_ebo_end(tree, branch);
		rs.setWireframe(sel, true, s1, e1);
	}
}

void ViewEditor::change(bool triangles)
{
	Mesh *m = scene.getMesh(0);
	int v = m->vertices.size();
	int e = m->triangles.size();
	int s = bt_generate_mesh(tree, &m->vertices[0], v, &m->triangles[0], e);

	if (s == 0) {
		m->vertices.resize(v + 1000);
		m->triangles.resize(e + 1000);
		v = m->vertices.size();
		e = m->triangles.size();
		rs.updateVertices(0, rs.MESH, &m->vertices[0], v, true);
		rs.updateTriangles(0, m, e, true);
		change(true);
		return;
	} else {
		int vs = bt_get_vbo_size(tree) * 6;
		int es = bt_get_ebo_size(tree);

		if (v > 4000 && e > 4000 && v - 2000 > vs && e - 2000 > es) {
			m->vertices.resize(vs + 1000);
			m->triangles.resize(es + 1000);
			v = m->vertices.size();
			e = m->triangles.size();
			rs.updateVertices(0, rs.MESH, &m->vertices[0], v, true);
			rs.updateTriangles(0, m, e, true);
		} else {
			float *vb = &m->vertices[0];
			rs.updateVertices(0, rs.MESH, vb, vs, false);
			if (triangles)
				rs.updateTriangles(0, m, es, false);
		}
	}

	updateWireframe();
	update();
}

void ViewEditor::changeResolution(int i)
{
	bt_set_resolution(tree, scene.getSelectedBranch(), i);
	change(true);
}

void ViewEditor::changeSections(int i)
{
	bt_set_cross_sections(tree, scene.getSelectedBranch(), i);
	change(true);
}

void ViewEditor::changeRadius(double d)
{
	bt_set_trunk_radius(tree, scene.getSelectedBranch(), d);
	change(false);
}

/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "editor.h"
#include "file_exporter.h"
#include "primitives.h"
#include "collision.h"
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtOpenGL/QGLFormat>

#define UNUSED(x) (void)(x)

Editor::Editor(QWidget *parent) : QOpenGLWidget(parent)
{
	ctrl = shift = midButton = false;
	camera.action = Camera::NONE;
	rs = NULL;
	setFocus();
}

Editor::~Editor()
{
	tmDeleteTree(tree);
}

void Editor::exportObject(const char *filename)
{
	FileExporter f;
	Entity *e = scene.getEntity(0);
	f.setVertices(&e->geometry.vertices[0], tmGetVBOSize(tree));
	f.setTriangles(&e->geometry.triangles[0], tmGetIBOSize(tree));
	f.exportObj(filename);
}

void Editor::setRenderSystem(RenderSystem *rs)
{
	this->rs = rs;
}

void Editor::initializeGL()
{
	initializeOpenGLFunctions();
	rs->init();

	ShaderInfo shaders[] = {
		{GL_VERTEX_SHADER, "shaders/flat.vert"},
		{GL_FRAGMENT_SHADER, "shaders/flat.frag"},
		{GL_VERTEX_SHADER, "shaders/basic.vert"},
		{GL_FRAGMENT_SHADER, "shaders/basic.frag"},
		{GL_VERTEX_SHADER, "shaders/solid.vert"},
		{GL_FRAGMENT_SHADER, "shaders/flat.frag"}
	};

	rs->loadShaders(&shaders[0], 2);
	rs->loadShaders(&shaders[2], 2);
	rs->loadShaders(&shaders[4], 2);

	initializeTree();
	initializeGrid();
}

void Editor::resizeGL(int width, int height)
{
	float aspectRatio = (float)width / (float)height;
	camera.setWindowSize(width, height);
	camera.setPerspective(45.0f, 0.1f, 100.0f, aspectRatio);
	glViewport(0, 0, width, height);
	paintGL();
}

void Editor::initializeGrid()
{
	GeometryComponent g = {};
	RenderComponent r = {};
	TMvec3 color = {0.46, 0.46, 0.46};
	TMvec3 sectionColor = {0.41, 0.41, 0.41};
	createGrid(g, 5, color, sectionColor);
	r.program = 0;
	r.type = RenderComponent::LINES;
	r.vertexRange[1] = g.vertices.size() / 6;
	rs->registerComponent(rs->load(g), r);
}

void Editor::initializeTree()
{
	const int es = 8000;
	const int vs = 8000;

	Entity e = {};
	RenderComponent *r;
	GeometryComponent *g;

	e.renderComponent.resize(1);
	r = &e.renderComponent[0];
	g = &e.geometry;

	g->attribs = 2;
	g->stride = sizeof(float) * 6;
	g->vertices.resize(vs);
	g->triangles.resize(es);
	r->program = 1;
	r->type = RenderComponent::TRIANGLES;

	tree = tmNewTree();
	tmSetRadius(tree, 0, 0.2f);
	tmSetResolution(tree, 0, 8);
	tmSetCrossSections(tree, 0, 12);
	tmSetMaxBranchDepth(tree, 1);
	tmSetCrownBaseHeight(tree, 2.0f);
	tmGenerateStructure(tree);
	tmGenerateMesh(tree, &g->vertices[0], vs, &g->triangles[0], es);

	r->triangleRange[1] = tmGetIBOSize(tree);
	r->vertexRange[1] = tmGetVBOSize(tree);

	rs->registerComponent(rs->load(*g), *r);
	scene.add(e);
	emit selectionChanged(tree, -1);
}

void Editor::keyPressEvent(QKeyEvent *event)
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

void Editor::keyReleaseEvent(QKeyEvent *event)
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

void Editor::selectBranch(int x, int y)
{
	Entity *selection;
	int branch;

	selection = scene.getSelected();
	if (selection != NULL)
		rs->setWireframe(selection->geometry.buffer, 0, 0, 0);

	selection = scene.setSelected(camera, x, y);
	branch = scene.setSelectedBranch(camera, x, y, tree);

	if (selection != NULL && branch >= 0) {
		int s = tmGetIBOStart(tree, branch);
		int e = tmGetIBOEnd(tree, branch);
		rs->setWireframe(selection->geometry.buffer, 0, s, e);
	}

	emit selectionChanged(tree, branch);
}

void Editor::mousePressEvent(QMouseEvent *event)
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

void Editor::mouseReleaseEvent(QMouseEvent *event)
{
	UNUSED(event);
	camera.action = Camera::NONE;
}

void Editor::mouseMoveEvent(QMouseEvent *event)
{
	QPoint point = event->pos();

	switch (camera.action) {
	case Camera::ZOOM:
		camera.zoom(point.y());
		break;
	case Camera::ROTATE:
		camera.setCoordinates(point.x(), point.y());
		break;
	case Camera::PAN:
		camera.setPan(point.x(), point.y());
		break;
	default:
		break;
	}

	update();
}

void Editor::paintGL()
{
	GlobalUniforms gu;
	gu.vp = camera.getVP();
	gu.cameraPosition = camera.getPosition();
	rs->render(gu);
}

void Editor::updateWireframe()
{
	Entity *selection = scene.getSelected();
	if (selection) {
		int branch = scene.getSelectedBranch();
		if (branch >= 0) {
			int s = tmGetIBOStart(tree, branch);
			int e = tmGetIBOEnd(tree, branch);
			rs->setWireframe(selection->geometry.buffer, 0, s, e);
		}
	}
}

void Editor::expandBuffer(GeometryComponent &g, RenderComponent &r)
{
	int status = 0;

	while (status == 0) {
		g.vertices.resize(g.vertices.size() + 1000);
		g.triangles.resize(g.triangles.size() + 1000);
		int v = g.vertices.size();
		int e = g.triangles.size();
		status = tmGenerateMesh(tree, &g.vertices[0], v,
				&g.triangles[0], e);
	}

	r.triangleRange[1] = tmGetIBOSize(tree);
	r.vertexRange[1] = tmGetVBOSize(tree);
	rs->registerComponent(rs->load(g, 0), r);
}

void Editor::change()
{
	Entity *e = scene.getEntity(0);
	GeometryComponent *g = &e->geometry;
	RenderComponent *r = &e->renderComponent[0];
	int vs = g->vertices.size();
	int es = g->triangles.size();
	int status = tmGenerateMesh(tree, &g->vertices[0], vs,
			&g->triangles[0], es);

	if (status == 0)
		expandBuffer(*g, *r);
	else  {
		int v = tmGetVBOSize(tree) * 6;
		int i = tmGetIBOSize(tree);

		if (vs > 8000 && es > 8000 && vs - 2000 > v && es - 2000 > i) {
			g->vertices.resize(v + 1000);
			g->triangles.resize(i + 1000);
			r->triangleRange[1] = tmGetIBOSize(tree);
			r->vertexRange[1] = tmGetVBOSize(tree);
			rs->registerComponent(rs->load(*g, 0), *r);
		} else {
			rs->updateVertices(0, &g->vertices[0], 0, v);
			rs->updateTriangles(0, &g->triangles[0], 0, i);
			rs->setVertexRange(0, 0, 0, v/6);
			rs->setTriangleRange(0, 0, 0, i);
			r->triangleRange[1] = tmGetIBOSize(tree);
			r->vertexRange[1] = tmGetVBOSize(tree);
		}
	}

	updateWireframe();
	update();
}

void Editor::changeResolution(int i)
{
	tmSetResolution(tree, scene.getSelectedBranch(), i);
	change();
}

void Editor::changeSections(int i)
{
	tmSetCrossSections(tree, scene.getSelectedBranch(), i);
	change();
}

void Editor::changeRadius(double d)
{
	tmSetRadius(tree, scene.getSelectedBranch(), d);
	change();
}

void Editor::changeRadiusCurve(vector<TMvec3> c)
{
	tmSetRadiusCurve(tree, scene.getSelectedBranch(), &c[0], c.size());
	change();
}

void Editor::changeBranchCurve(vector<TMvec3> c)
{
	tmSetBranchCurve(tree, scene.getSelectedBranch(), &c[0], c.size());
	change();
}

void Editor::changeBranchDensity(double d)
{
	tmSetBranchDensity(tree, scene.getSelectedBranch(), d);
	change();
}

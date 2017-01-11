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
#include <cmath>
#include <limits>

#define UNUSED(x) (void)(x)

CurveEditor::CurveEditor(QWidget *parent) : QOpenGLWidget(parent)
{
	enabled = false;
	point = std::numeric_limits<size_t>::max();
}

QSize CurveEditor::sizeHint() const
{
	return QSize(300, 40);
}

void CurveEditor::initializeGL()
{
	initializeOpenGLFunctions();
	rs.init();
	onFloat();
	connect(parent(), SIGNAL(topLevelChanged(bool)), this,
			SLOT(onFloat()));

	ShaderInfo shaders[] = {
		{GL_VERTEX_SHADER, "shaders/flat.vert"},
		{GL_FRAGMENT_SHADER, "shaders/flat.frag"},
	};
	rs.loadShaders(&shaders[0], 2);

	createInterface();
}

void CurveEditor::onFloat()
{
	QList<QTabBar *> l = parent()->parent()->findChildren<QTabBar *>();
	if (l.size() > 0)
		l[0]->setDrawBase(false);
}

void CurveEditor::createInterface()
{
	int offset = 0;
	int buffer;

	ui.renderComponent.resize(4);
	offset = createBackground(offset);
	offset = createControlLines(offset);
	offset = createCurve(offset);
	ui.renderComponent[2].hidden = true;
	ui.renderComponent[3].hidden = true;

	buffer = rs.load(ui.geometry);
	for (int i = 3; i >= 0; i--)
		rs.registerComponent(buffer, ui.renderComponent[i]);
}

int CurveEditor::createBackground(int offset)
{
	{
		TMvec3 color = {0.3f, 0.3f, 0.3f};
		TMvec3 sectionColor = {0.3f, 0.3f, 0.3f};
		RenderComponent *r = &ui.renderComponent[1];
		createGrid(ui.geometry, 2, color, sectionColor, (TMmat4){
				1.0f/4.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f/4.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f/4.0f, 0.0f,
				0.5f, 0.0f, 0.5f, 1.0f});
		r->type = RenderComponent::LINES;
		r->vertexRange[0] = offset;
		r->vertexRange[1] = ui.geometry.vertices.size() / 6;
		offset = r->vertexRange[1];
	}

	{
		TMvec3 a = {1.0f, 0.0f, 0.0f};
		TMvec3 b = {0.0f, 0.0f, 1.0f};
		TMvec3 center = {0.0f, 0.2f, 0.0f};
		RenderComponent *r = &ui.renderComponent[0];
		createPlane(ui.geometry, a, b, center);
		r->type = RenderComponent::TRIANGLES;
		r->vertexRange[0] = offset;
		r->vertexRange[1] = ui.geometry.vertices.size() / 6;
		r->triangleRange[1] = ui.geometry.triangles.size();
		return r->vertexRange[1];
	}
}

int CurveEditor::createControlLines(int offset)
{
	RenderComponent *r = &ui.renderComponent[2];
	createLine(ui.geometry, controls, {0.6f, 0.6f, 0.6f});
	r->type = RenderComponent::LINES;
	r->vertexRange[0] = offset;
	r->vertexRange[1] = ui.geometry.vertices.size() / 6;
	r->pointRange[0] = offset;
	r->pointRange[1] = ui.geometry.vertices.size() / 6;
	return r->vertexRange[1];
}

int CurveEditor::createCurve(int offset)
{
	RenderComponent *r = &ui.renderComponent[3];
	createPath(ui.geometry, controls, 100, {0.2f, 0.46f, 0.6f});
	r->type = RenderComponent::LINE_STRIP;
	r->vertexRange[0] = offset;
	r->vertexRange[1] = ui.geometry.vertices.size() / 6;
	return r->vertexRange[1];
}

void CurveEditor::resizeGL(int width, int height)
{
	glViewport(0, 0, width, height);
	this->width = width;
	this->height = height;
}

void CurveEditor::mousePressEvent(QMouseEvent *event)
{
	if (!enabled || event->button() != Qt::LeftButton)
		return;

	QPoint p = event->pos();
	for (unsigned i = 0; i < controls.size(); i++) {
		int x = (controls[i].x * 0.9f + 0.05f) * width;
		int y = (height - (controls[i].z * 0.9f + 0.05f) * height);
		if (sqrt(pow(p.x() - x, 2) + pow(p.y() - y, 2)) < 8) {
			point = i;
			insertIndex = std::numeric_limits<size_t>::max();
			x = p.x();
			y = p.y();
		}
	}
}

void toDeviceCoordinates(float &x, float &y, int width, int height)
{
	x = x/(width - 1.0f)*10.0f/9.0f - 1.0f/18.0f;
	y = (1.0f - y/(height - 1.0f))*10.0f/9.0f - 1.0f/18.0f;
}

void CurveEditor::insertCurve(int i, float x, float y)
{
	float minX = x-0.1f > controls[i-3].x ? x-0.1f : controls[i-3].x;
	float maxX = x+0.1f < controls[i].x ? x+0.1f : controls[i].x;

	if (x <= controls[i].x && x >= controls[i-1].x)
		return;
	if (x >= controls[i-3].x && x <= controls[i-2].x)
		return;

	curve[0] = {minX, -0.3f, y};
	curve[1] = {x, -0.3f, y};
	curve[2] = {x, -0.3f, y};
	curve[3] = {maxX, -0.3f, y};
	controls.insert(controls.begin()+i-1, curve, &curve[4]);

	updateCurve();
	emit curveChanged(controls, curveName);
	update();
}

void CurveEditor::mouseDoubleClickEvent(QMouseEvent *event)
{
	if (!enabled || event->button() != Qt::LeftButton)
		return;

	QPoint p = event->pos();
	float x = p.x();
	float y = p.y();
	toDeviceCoordinates(x, y, width, height);

	for (size_t i = 3; i < controls.size(); i += 4)
		if (controls[i].x > x) {
			insertCurve(i, x, y);
			break;
		}
}

void CurveEditor::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
		point = std::numeric_limits<size_t>::max();
}

void CurveEditor::mouseMoveEvent(QMouseEvent *event)
{
	QPoint p = event->pos();
	float x = p.x();
	float y = p.y();
	const size_t max = std::numeric_limits<size_t>::max();
	bool removed = insertIndex != 0 && insertIndex != max;
	toDeviceCoordinates(x, y, width, height);

	if (point == std::numeric_limits<size_t>::max())
		return;
	if (removed && !reinsertCurve(x))
		return;

	if (point == 0)
		placeTerminalControl(true, y);
	else if (point == controls.size() - 1)
		placeTerminalControl(false, y);
	else if (point % 4 == 0) {
		if (!omitCurve(x))
			placeOuterControl(x, y);
	} else
		placeInnerControl(x, y);

	if (insertIndex == std::numeric_limits<size_t>::max())
		drawCurve();
	else
		updateCurve();

	emit curveChanged(controls, curveName);
	update();
}

void CurveEditor::updateCurve()
{
	int start = ui.renderComponent[2].vertexRange[0];
	ui.geometry.vertices.resize(start * 6);
	start = createControlLines(start);
	start = createCurve(start);
	ui.renderComponent[2].hidden = false;
	ui.renderComponent[3].hidden = false;
	rs.load(ui.geometry, 0);
	for (int i = 3; i >= 0; i--)
		rs.registerComponent(0, ui.renderComponent[i]);
}

void CurveEditor::drawCurve()
{
	int start = ui.renderComponent[2].vertexRange[0] * 6;
	GeometryComponent g;
	createLine(g, controls, {0.6, 0.6, 0.6});
	createPath(g, controls, 100, {.2f, 0.46f, 0.6f});
	rs.updateVertices(0, &g.vertices[0], start, g.vertices.size());
}

bool CurveEditor::reinsertCurve(float x)
{
	if (x > controls[insertIndex-2].x) {
		auto index = controls.begin() + insertIndex;
		controls.insert(index, &curve[0], &curve[4]);
		updateCurve();
		insertIndex = std::numeric_limits<size_t>::max();
		return true;
	} else
		return false;
}

bool CurveEditor::omitCurve(float x)
{
	if (x < controls[point-4].x || x > controls[point+3].x) {
		insertIndex = point - 2;
		auto start = controls.begin() + insertIndex;
		auto end = controls.begin() + insertIndex + 4;
		memcpy(curve, &controls[insertIndex], sizeof(TMvec3)*4);
		controls.erase(start, end);
		return true;
	} else
		return false;
}

void CurveEditor::placeOuterControl(float x, float y)
{
	float dx1 = controls[point-2].x-controls[point].x;
	float dy1 = controls[point-2].z-controls[point].z;
	float dx2 = controls[point+1].x-controls[point].x;
	float dy2 = controls[point+1].z-controls[point].z;

	if (x < controls[point-3].x)
		x = controls[point-3].x;
	if (x > controls[point+2].x)
		x = controls[point+2].x;
	if (x + dx1 < controls[point-4].x)
		x = controls[point-4].x - dx1;
	if (x + dx2 > controls[point+3].x)
		x = controls[point+3].x - dx2;
	if (y + dy1 > 1.0f)
		y = 1.0f - dy1;
	if (y + dy1 < 0.0f)
		y = -dy1;
	if (y + dy2 < 0.0f)
		y = -dy2;
	if (y + dy2 > 1.0f)
		y = 1.0f - dy2;
	if (y > 1.0f)
		y = 1.0f;
	if (y < 0.0f)
		y = 0.0f;

	controls[point+1].x = x + dx2;
	controls[point+1].z = y + dy2;
	controls[point+0].x = x;
	controls[point+0].z = y;
	controls[point-1].x = x;
	controls[point-1].z = y;
	controls[point-2].x = x + dx1;
	controls[point-2].z = y + dy1;
}

void CurveEditor::placeInnerControl(float x, float y)
{
	int l = (point-1)%4 == 0 ? 0 : -1;

	if (x < controls[point-1+l].x)
		x = controls[point-1+l].x;
	else if (x > controls[point+2+l].x)
		x = controls[point+2+l].x;
	if (y > 1.0f)
		y = 1.0f;
	else if (y < 0.0f)
		y = 0.0f;

	controls[point].x = x;
	controls[point].z = y;
}

void CurveEditor::placeTerminalControl(bool first, float y)
{
	int adjacent = first ? 1 : -1;
	float dy = controls[point + adjacent].z - controls[point].z;

	if (y + dy > 1.0f)
		y = 1.0f - dy;
	else if (y + dy < 0.0f)
		y = -dy;
	if (y > 1.0f)
		y = 1.0f;
	else if (y < 0.0f)
		y = 0.0f;

	controls[point + adjacent].z = y + dy;
	controls[point].z = y;
}

void CurveEditor::paintGL()
{
	GlobalUniforms gu;
	gu.vp = (TMmat4){
			1.8f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.9f, 0.0f,
			0.0f, 1.8f, 0.0f, 0.0f,
			-0.9f, -0.9f, 0.0f, 1.0f};
	rs.render(gu, 0.3f);
}

void CurveEditor::setCurve(vector<TMvec3> controls, QString name)
{
	parentWidget()->setWindowTitle(name + " curve");
	this->controls = controls;
	this->curveName = name;
	updateCurve();
	update();
}

void CurveEditor::setEnabled(bool enabled)
{
	if (enabled && curveName.size() > 0)
		parentWidget()->setWindowTitle(curveName + " curve");
	else
		parentWidget()->setWindowTitle("Curve");

	this->enabled = enabled;
	rs.setHidden(0, 0, !enabled);
	rs.setHidden(0, 1, !enabled);
	update();
}

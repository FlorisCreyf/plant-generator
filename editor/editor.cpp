/* TreeMaker: 3D tree model editor
 * Copyright (C) 2016-2017  Floris Creyf
 *
 * TreeMaker is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TreeMaker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "editor.h"
#include "file_exporter.h"
#include "geometry.h"
#include "collision.h"
#include "vector.h"
#include <math.h>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtOpenGL/QGLFormat>

#define UNUSED(x) (void)(x)

Editor::Editor(SharedResources *shared, QWidget *parent) : QOpenGLWidget(parent)
{
	this->shared = shared;
	setFocus();
}

Editor::~Editor()
{
	tmDeleteTree(tree);
}

void Editor::exportObject(const char *filename)
{
	FileExporter f;
	f.setVertices(&vertices[0], tmGetVBOSize(tree));
	f.setTriangles(&indices[0], tmGetIBOSize(tree));
	f.exportObj(filename);
}

void Editor::initializeGL()
{
	initializeOpenGLFunctions();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	shared->create();
	initializeTree();
	initializeGrid();
	intializeLines();
}

void Editor::resizeGL(int width, int height)
{
	float aspectRatio = (float)width / (float)height;
	camera.setWindowSize(width, height);
	camera.setPerspective(45.0f, 0.1f, 100.0f, aspectRatio);
	axis.setScale(height);
	glViewport(0, 0, width, height);
	paintGL();
}

void Editor::initializeGrid()
{
	Geometry geom;
	gridInfo = geom.addGrid(5, {0.41, 0.41, 0.41}, {0.46, 0.46, 0.46});
	axis.create(geom);

	glGenVertexArrays(1, &bufferSets[0].vao);
	glBindVertexArray(bufferSets[0].vao);
	glGenBuffers(2, bufferSets[0].buffers);

	glBindBuffer(GL_ARRAY_BUFFER, bufferSets[0].buffers[0]);
	graphics::load(GL_ARRAY_BUFFER, geom.vertices, GL_STATIC_DRAW);
	graphics::setVertexFormat(geom.getVertexFormat());

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferSets[0].buffers[1]);
	graphics::load(GL_ELEMENT_ARRAY_BUFFER, geom.indices, GL_STATIC_DRAW);
}

void Editor::initializeTree()
{
	glGenVertexArrays(1, &bufferSets[1].vao);
	glBindVertexArray(bufferSets[1].vao);
	glGenBuffers(2, bufferSets[1].buffers);

	vertices.resize(8000);
	indices.resize(8000);

	tree = tmNewTree();
	tmSetRadius(tree, 0, 0.2f);
	tmSetResolution(tree, 0, 8);
	tmSetCrossSections(tree, 0, 12);
	tmSetMaxBranchDepth(tree, 1);
	tmSetCrownBaseHeight(tree, 2.0f);
	tmGenerateStructure(tree);
	tmGenerateMesh(tree, &vertices[0], 8000, &indices[0], 8000);

	treeInfo.type = GL_TRIANGLES;
	treeInfo.start[0] = treeInfo.start[1] = 0;
	treeInfo.count[0] = tmGetVBOSize(tree);
	treeInfo.count[1] = tmGetIBOSize(tree);

	glBindBuffer(GL_ARRAY_BUFFER, bufferSets[1].buffers[0]);
	graphics::load(GL_ARRAY_BUFFER, vertices, GL_DYNAMIC_DRAW);
	graphics::setVertexFormat(graphics::VERTEX_NORMAL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferSets[1].buffers[1]);
	graphics::load(GL_ELEMENT_ARRAY_BUFFER, indices, GL_DYNAMIC_DRAW);

	emit selectionChanged(tree, -1);
}

void Editor::intializeLines()
{
	GLsizeiptr size = sizeof(TMvec3) * maxLines;
	glGenVertexArrays(1, &bufferSets[2].vao);
	glBindVertexArray(bufferSets[2].vao);
	glGenBuffers(1, bufferSets[2].buffers);
	glBindBuffer(GL_ARRAY_BUFFER, bufferSets[2].buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	graphics::setVertexFormat(graphics::VERTEX_COLOR);
}

void Editor::updateLines(int branch)
{
	Geometry geom;
	const int size = tmGetBranchPathSize(tree, 0);
	std::vector<TMvec3> points(size);
	tmGetBranchPath(tree, branch, &points[0]);
	lineInfo = geom.addLine(points, {0.76, 0.54, 0.29});

	glBindBuffer(GL_ARRAY_BUFFER, bufferSets[2].buffers[0]);
	if (points.size() > maxLines) {
		maxLines *= 2;
		GLsizeiptr size = sizeof(TMvec3) * maxLines;
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
		graphics::setVertexFormat(geom.getVertexFormat());
	} else if (maxLines > minLines && points.size()/2 < maxLines ) {
		maxLines /= 2;
		GLsizeiptr size = sizeof(TMvec3) * maxLines;
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
		graphics::setVertexFormat(geom.getVertexFormat());
	}

	graphics::update(GL_ARRAY_BUFFER, 0, geom.vertices);
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
	case Qt::Key_Tab:
		break;
	}
}

bool Editor::event(QEvent *e)
{
	if (e->type() == QEvent::KeyPress) {
		QKeyEvent *k = static_cast<QKeyEvent *>(e);
		keyPressEvent(k);
		return true;
	}
	return QWidget::event(e);
}

void Editor::selectBranch(int x, int y)
{
	TMvec3 direction = camera.getRayDirection(x, y);
	TMvec3 origin = camera.getPosition();
	TMaabb box = tmGetBoundingBox(tree, 0);
	int branch = 0;

	while (box.x1 != box.x2) {
		float t = tmIntersectsAABB(origin, direction, box);
		if (t != 0.0f) {
			int start = tmGetIBOStart(tree, branch);
			int end = tmGetIBOEnd(tree, branch);
			selection.start[1] = start;
			selection.count[1] = end - start;
			selectedBranch = branch;
			updateLines(branch);
			emit selectionChanged(tree, branch);
			return;
		}
		box = tmGetBoundingBox(tree, ++branch);
	}

	selectedBranch = -1;
	emit selectionChanged(tree, selectedBranch);
}

void Editor::selectPoint(int x, int y)
{
	int size = tmGetBranchPathSize(tree, 0);
	std::vector<TMvec3> points(size);
	tmGetBranchPath(tree, selectedBranch, &points[0]);

	for (int i = 0; i < size; i++) {
		TMvec3 point = points[i];
		point = camera.toScreenSpace(points[i]);
		if (sqrt(pow(point.x - x, 2) + pow(point.y - y, 2)) < 10) {
			selectedPoint = i;
			return;
		}
	}

	selectedPoint = -1;
}

void Editor::selectAxis(int x, int y)
{
	TMvec3 c = tmGetBranchPoint(tree, selectedBranch, selectedPoint);
	TMvec3 o = camera.getPosition();
	TMvec3 d = camera.getRayDirection(x, y);
	TMvec3 s = camera.toScreenSpace(c);

	clickOffset[0] = s.x - x;
	clickOffset[1] = s.y - y;

	axis.pickAxis(c, {o, d});
}

void Editor::mousePressEvent(QMouseEvent *event)
{
	QPoint p = event->pos();
	midButton = false;

	if (event->button() == Qt::RightButton) {
		int prevBranch = selectedBranch;

		if (selectedBranch >= 0)
			selectPoint(p.x(), p.y());
		if (selectedPoint == -1)
			selectBranch(p.x(), p.y());
		if (selectedBranch != prevBranch)
			selectedPoint = -1;

		update();
	} else if (event->button() == Qt::MidButton) {
		midButton = true;
		camera.setStartCoordinates(p.x(), p.y());
		if (ctrl && !shift)
			camera.action = Camera::ZOOM;
		else if (shift && !ctrl)
			camera.action = Camera::PAN;
		else
			camera.action = Camera::ROTATE;
	} else if (event->button() == Qt::LeftButton) {
		if (selectedPoint >= 0 && selectedBranch >= 0)
			selectAxis(p.x(), p.y());
	}

	setFocus();
}

void Editor::mouseReleaseEvent(QMouseEvent *event)
{
	UNUSED(event);
	if (event->button() == Qt::MidButton)
		camera.action = Camera::NONE;
	if (event->button() == Qt::LeftButton)
		axis.clearLastSelected();
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

	if (axis.getLastSelected() != Axis::NONE)
		movePoint(point.x(), point.y());

	update();
}

void Editor::movePoint(int x, int y)
{
	x += clickOffset[0];
	y += clickOffset[1];

	TMray r = {camera.getPosition(), camera.getRayDirection(x, y)};
	TMvec3 p = tmGetBranchPoint(tree, selectedBranch, selectedPoint);
	p = axis.move(axis.getLastSelected(), r, camera.getDirection(), p);
	tmSetBranchPoint(tree, selectedBranch, p, selectedPoint);

	{
		int vs = vertices.size();
		int es = indices.size();
		tmGenerateMesh(tree, &vertices[0], vs, &indices[0], es);
		updateLines(selectedBranch);
		updateBuffers();
	}
}

void Editor::paintGL()
{
	TMmat4 vp = camera.getVP();
	TMvec3 cp = camera.getPosition();

	glDepthFunc(GL_LEQUAL);
	glClearColor(0.5f, 0.5f, 0.5f, 1.0);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shared->getProgramName(shared->MODEL_SHADER));
	glBindVertexArray(bufferSets[1].vao);
	glUniformMatrix4fv(0, 1, GL_FALSE, &vp.m[0][0]);
	glUniform4f(1, cp.x, cp.y, cp.z, 0.0f);
	glDrawElements(GL_TRIANGLES, treeInfo.count[1], GL_UNSIGNED_SHORT, 0);

	if (selectedBranch != -1)
		paintSelectionWireframe();

	glUseProgram(shared->getProgramName(shared->FLAT_SHADER));
	glBindVertexArray(bufferSets[0].vao);
	glUniformMatrix4fv(0, 1, GL_FALSE, &vp.m[0][0]);
	glDrawArrays(gridInfo.type, gridInfo.start[0], gridInfo.count[0]);

	if (selectedBranch != -1) {
		paintSelectionLines();
		if (selectedPoint != -1)
			paintAxis();
	}

	glFlush();
}

/* This method assumes that the proper vertex array is already set. */
void Editor::paintSelectionWireframe()
{
	TMmat4 vp = camera.getVP();
	GLvoid *p = (GLvoid *)(selection.start[1] * sizeof(unsigned short));

	glUseProgram(shared->getProgramName(shared->WIREFRAME_SHADER));
	glUniformMatrix4fv(0, 1, GL_FALSE, &vp.m[0][0]);
	glPolygonOffset(-0.1f, -0.1f);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawElements(GL_TRIANGLES, selection.count[1], GL_UNSIGNED_SHORT, p);
	glPolygonOffset(0.0f, 0.0f);
	glDisable(GL_POLYGON_OFFSET_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

/* This method assumes that FLAT_SHADER is already set. */
void Editor::paintSelectionLines()
{
	TMmat4 vp = camera.getVP();

	glDepthFunc(GL_ALWAYS);
	glPointSize(8);

	glBindVertexArray(bufferSets[2].vao);
	glBindTexture(GL_TEXTURE_2D, shared->getTextureName(shared->DOT_TEX));
	glUseProgram(shared->getProgramName(shared->POINT_SHADER));
	glUniformMatrix4fv(0, 1, GL_FALSE, &vp.m[0][0]);
	glDrawArrays(GL_POINTS, lineInfo.start[0], lineInfo.count[0]);

	glUseProgram(shared->getProgramName(shared->LINE_SHADER));
	glBindVertexArray(bufferSets[2].vao);
	glUniformMatrix4fv(0, 1, GL_FALSE, &vp.m[0][0]);
	glUniform2f(1, QWidget::width(), QWidget::height());
	glDrawArrays(lineInfo.type, lineInfo.start[0], lineInfo.count[0]);
}

void Editor::paintAxis()
{
	TMmat4 vp = camera.getVP();
	TMvec3 cp = camera.getPosition();
	graphics::Fragment lines = axis.getLineFragment();
	graphics::Fragment arrows = axis.getArrowFragment();
	GLvoid *p = BUFFER_OFFSET(arrows.start[1]);
	TMvec3 center = tmGetBranchPoint(tree, selectedBranch, selectedPoint);
	TMmat4 mat = axis.getModelMatrix(center, cp);
	mat = tmMultMat4(&vp, &mat);

	glClear(GL_DEPTH_BUFFER_BIT);
	glDepthFunc(GL_LEQUAL);
	glUseProgram(shared->getProgramName(shared->FLAT_SHADER));
	glBindVertexArray(bufferSets[0].vao);
	glUniformMatrix4fv(0, 1, GL_FALSE, &mat.m[0][0]);
	glDrawArrays(lines.type, lines.start[0], lines.count[0]);
	glDrawElements(GL_TRIANGLES, arrows.count[1], GL_UNSIGNED_SHORT, p);
}

void Editor::updateSelection()
{
	if (selectedBranch != -1) {
		int start = tmGetIBOStart(tree, selectedBranch);
		int end = tmGetIBOEnd(tree, selectedBranch);
		selection.start[1] = start;
		selection.count[1] = end - start;
	}
}

void Editor::expandBuffers()
{
	int status = 0;

	while (status == 0) {
		vertices.resize(vertices.size() * 2);
		indices.resize(indices.size() * 2);
		int v = vertices.size();
		int e = indices.size();
		status = tmGenerateMesh(tree, &vertices[0], v, &indices[0], e);
	}

	createBuffers();
	treeInfo.count[0] = tmGetVBOSize(tree);
	treeInfo.count[1] = tmGetIBOSize(tree);
}

void Editor::createBuffers()
{
	glBindBuffer(GL_ARRAY_BUFFER, bufferSets[1].buffers[0]);
	graphics::load(GL_ARRAY_BUFFER, vertices, GL_DYNAMIC_DRAW);
	graphics::setVertexFormat(graphics::VERTEX_NORMAL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferSets[1].buffers[1]);
	graphics::load(GL_ELEMENT_ARRAY_BUFFER, indices, GL_DYNAMIC_DRAW);
}

void Editor::updateBuffers()
{
	int size = treeInfo.count[0];
	size *= graphics::getSize(graphics::VERTEX_NORMAL) * sizeof(float);
	glBindBuffer(GL_ARRAY_BUFFER, bufferSets[1].buffers[0]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, size, &vertices[0]);

	size = treeInfo.count[1] * sizeof(unsigned short);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferSets[1].buffers[1]);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, size, &indices[0]);
}

void Editor::change()
{
	int vs = vertices.size();
	int es = indices.size();
	int status = tmGenerateMesh(tree, &vertices[0], vs, &indices[0], es);

	if (status == 0)
		expandBuffers();
	else  {
		int v = tmGetVBOSize(tree) * 6;
		int i = tmGetIBOSize(tree);

		if (vs > 8000 && es > 8000 && vs/2 > v && es/2 > i) {
			vertices.resize(v + 1000);
			indices.resize(i + 1000);
			treeInfo.count[0] = tmGetVBOSize(tree);
			treeInfo.count[1] = tmGetIBOSize(tree);
			createBuffers();
		} else {
			treeInfo.count[0] = tmGetVBOSize(tree);
			treeInfo.count[1] = tmGetIBOSize(tree);
			updateBuffers();
		}
	}

	updateSelection();
	update();
}

void Editor::changeResolution(int i)
{
	tmSetResolution(tree, selectedBranch, i);
	change();
}

void Editor::changeSections(int i)
{
	tmSetCrossSections(tree, selectedBranch, i);
	change();
}

void Editor::changeRadius(double d)
{
	tmSetRadius(tree, selectedBranch, d);
	change();
}

void Editor::changeRadiusCurve(std::vector<TMvec3> c)
{
	tmSetRadiusCurve(tree, selectedBranch, &c[0], c.size());
	change();
}

void Editor::changeBranchCurve(std::vector<TMvec3> c)
{
	tmSetBranchCurve(tree, selectedBranch, &c[0], c.size());
	change();
}

void Editor::changeBranchDensity(double d)
{
	tmSetBranchDensity(tree, selectedBranch, d);
	change();
}

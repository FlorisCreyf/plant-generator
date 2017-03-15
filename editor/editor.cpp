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
#include "intersection.h"
#include "math.h"
#include <cmath>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtOpenGL/QGLFormat>

#define UNUSED(x) (void)(x)

using namespace treemaker;

Editor::Editor(SharedResources *shared, QWidget *parent) : QOpenGLWidget(parent)
{
	this->shared = shared;
	setFocus();
}

void Editor::exportObject(const char *filename)
{
	FileExporter f;
	f.setVertices(tree.getVertices(), tree.getVertexCount());
	f.setTriangles(tree.getIndices(), tree.getIndexCount());
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
	gridInfo = geom.addGrid(5, {0.41, 0.41, 0.41}, {0.3, 0.3, 0.3});
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

	tree.setRadius(0, 0.2f);
	tree.setResolution(0, 8);
	tree.setGeneratedPathSize(0, 12);
	tree.setMaxStemDepth(1);
	tree.generateTree();

	treeInfo.type = GL_TRIANGLES;
	treeInfo.start[0] = treeInfo.start[1] = 0;
	treeInfo.count[0] = tree.getVertexCount();
	treeInfo.count[1] = tree.getIndexCount();

	change();
	emit selectionChanged(tree, -1);
}

void Editor::intializeLines()
{
	GLsizeiptr size = sizeof(Vec3) * maxLines;
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
	const size_t size = tree.getPathSize(branch);
	std::vector<Vec3> points(size);
	tree.getPath(branch, &points[0]);
	lineInfo = geom.addLine(points, {0.76, 0.54, 0.29});

	glBindBuffer(GL_ARRAY_BUFFER, bufferSets[2].buffers[0]);
	if (points.size() > maxLines) {
		maxLines *= 2;
		GLsizeiptr size = sizeof(Vec3) * maxLines;
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
		graphics::setVertexFormat(geom.getVertexFormat());
	} else if (maxLines > minLines && points.size()/2 < maxLines ) {
		maxLines /= 2;
		GLsizeiptr size = sizeof(Vec3) * maxLines;
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

void Editor::selectStem(int x, int y)
{
	Ray ray;
	ray.direction = camera.getRayDirection(x, y);
	ray.origin = camera.getPosition();
	Aabb box = tree.getBoundingBox(0);
	unsigned branch = tree.getStemName(0);
	
	for (unsigned i = 0; box.a.x != box.b.x;) {
		
		float t = intersectsAABB(ray, box);
		if (t != 0.0f) {
			auto location = tree.getStemLocation(branch);
			selection.start[1] = location.indexStart;
			selection.count[1] = location.indexCount;
			selectedStem = branch;
			updateLines(branch);
			emit selectionChanged(tree, branch);
			return;
		}
		branch = tree.getStemName(++i);
		box = tree.getBoundingBox(branch);
	}

	selectedStem = -1;
	emit selectionChanged(tree, selectedStem);
}

void Editor::selectPoint(int x, int y)
{
	int size = tree.getPathSize(selectedStem);
	std::vector<Vec3> points(size);
	tree.getPath(selectedStem, &points[0]);

	for (int i = 0; i < size; i++) {
		Vec3 point = camera.toScreenSpace(points[i]);
		float sx = std::pow(point.x - x, 2);
		float sy = std::pow(point.y - y, 2);

		if (std::sqrt(sx + sy) < 10) {
			selectedPoint = i;
			return;
		}
	}

	selectedPoint = -1;
}

void Editor::selectAxis(int x, int y)
{
	std::vector<Vec3> path(tree.getPathSize(selectedStem));
	tree.getPath(selectedStem, &path[0]);
	Vec3 c = path[selectedPoint];
	Vec3 o = camera.getPosition();
	Vec3 d = camera.getRayDirection(x, y);
	Vec3 s = camera.toScreenSpace(c);

	clickOffset[0] = s.x - x;
	clickOffset[1] = s.y - y;

	axis.pickAxis(c, {o, d});
}

void Editor::mousePressEvent(QMouseEvent *event)
{
	QPoint p = event->pos();
	midButton = false;

	if (event->button() == Qt::RightButton) {
		int prevBranch = selectedStem;

		if (selectedStem >= 0)
			selectPoint(p.x(), p.y());
		if (selectedPoint == -1)
			selectStem(p.x(), p.y());
		if (selectedStem != prevBranch)
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
		if (selectedPoint >= 0 && selectedStem >= 0)
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
		axis.clearSelected();
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

	if (axis.getSelected() != Axis::NONE)
		movePoint(point.x(), point.y());

	update();
}

void Editor::movePoint(int x, int y)
{
	x += clickOffset[0];
	y += clickOffset[1];

	Ray ray = {camera.getPosition(), camera.getRayDirection(x, y)};
	Vec3 direction = camera.getDirection();
	
	if (selectedPoint == 0 && !tree.isLateral(selectedStem)) {
		Vec3 loc = tree.getLocation(selectedStem);
		loc = axis.move(axis.getSelected(), ray, direction, loc);
		tree.setLocation(selectedStem, loc);
	} else {
		Vec3 loc;
		std::vector<Vec3> path(tree.getPathSize(selectedStem));
		tree.getPath(selectedStem, &path[0]);
		loc = path[selectedPoint];
		loc = axis.move(axis.getSelected(), ray, direction, loc);
		path[selectedPoint] = loc;
		tree.setPath(selectedStem, &path[0], path.size());
	}
	
	{
		updateLines(selectedStem);
		change();
	}
}

void Editor::paintGL()
{
	Mat4 vp = camera.getVP();
	Vec3 cp = camera.getPosition();

	glDepthFunc(GL_LEQUAL);
	glClearColor(0.2f, 0.2f, 0.2f, 1.0);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shared->getProgramName(shared->MODEL_SHADER));
	glBindVertexArray(bufferSets[1].vao);
	glUniformMatrix4fv(0, 1, GL_FALSE, &vp[0][0]);
	glUniform4f(1, cp.x, cp.y, cp.z, 0.0f);
	glDrawElements(GL_TRIANGLES, treeInfo.count[1], GL_UNSIGNED_INT, 0);

	if (selectedStem != -1)
		paintSelectionWireframe();

	glUseProgram(shared->getProgramName(shared->FLAT_SHADER));
	glBindVertexArray(bufferSets[0].vao);
	glUniformMatrix4fv(0, 1, GL_FALSE, &vp[0][0]);
	glDrawArrays(gridInfo.type, gridInfo.start[0], gridInfo.count[0]);

	if (selectedStem != -1) {
		paintSelectionLines();
		if (selectedPoint != -1)
			paintAxis();
	}

	glFlush();
}

/** This method assumes that the proper vertex array is already set. */
void Editor::paintSelectionWireframe()
{
	Mat4 vp = camera.getVP();
	GLvoid *p = (GLvoid *)(selection.start[1] * sizeof(unsigned int));

	glUseProgram(shared->getProgramName(shared->WIREFRAME_SHADER));
	glUniformMatrix4fv(0, 1, GL_FALSE, &vp[0][0]);
	glPolygonOffset(-0.1f, -0.1f);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawElements(GL_TRIANGLES, selection.count[1], GL_UNSIGNED_INT, p);
	glPolygonOffset(0.0f, 0.0f);
	glDisable(GL_POLYGON_OFFSET_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

/** This method assumes that FLAT_SHADER is already set. */
void Editor::paintSelectionLines()
{
	Mat4 vp = camera.getVP();

	glDepthFunc(GL_ALWAYS);
	glPointSize(8);

	glBindVertexArray(bufferSets[2].vao);
	glBindTexture(GL_TEXTURE_2D, shared->getTextureName(shared->DOT_TEX));
	glUseProgram(shared->getProgramName(shared->POINT_SHADER));
	glUniformMatrix4fv(0, 1, GL_FALSE, &vp[0][0]);
	glDrawArrays(GL_POINTS, lineInfo.start[0], lineInfo.count[0]);

	glUseProgram(shared->getProgramName(shared->LINE_SHADER));
	glBindVertexArray(bufferSets[2].vao);
	glUniformMatrix4fv(0, 1, GL_FALSE, &vp[0][0]);
	glUniform2f(1, QWidget::width(), QWidget::height());
	glDrawArrays(lineInfo.type, lineInfo.start[0], lineInfo.count[0]);
}

void Editor::paintAxis()
{
	Mat4 vp = camera.getVP();
	Vec3 cp = camera.getPosition();
	graphics::Fragment lines = axis.getLineFragment();
	graphics::Fragment arrows = axis.getArrowFragment();
	GLvoid *p = BUFFER_OFFSET(arrows.start[1]);
	Vec3 center;

	{
		std::vector<Vec3> path(tree.getPathSize(selectedStem));
		tree.getPath(selectedStem, &path[0]);
		center = path[selectedPoint];
	}

	vp = vp * axis.getModelMatrix(center, cp);

	glClear(GL_DEPTH_BUFFER_BIT);
	glDepthFunc(GL_LEQUAL);
	glUseProgram(shared->getProgramName(shared->FLAT_SHADER));
	glBindVertexArray(bufferSets[0].vao);
	glUniformMatrix4fv(0, 1, GL_FALSE, &vp[0][0]);
	glDrawArrays(lines.type, lines.start[0], lines.count[0]);
	glDrawElements(GL_TRIANGLES, arrows.count[1], GL_UNSIGNED_SHORT, p);
}

void Editor::updateSelection()
{
	if (selectedStem != -1) {
		auto location = tree.getStemLocation(selectedStem);
		selection.start[1] = location.indexStart;
		selection.count[1] = location.indexCount;
	}
}

void Editor::change()
{
	bool resized = tree.generateMesh();
	const float *v = tree.getVertices();
	const unsigned *i = tree.getIndices();
	const size_t vs = tree.getVertexCapacity() * sizeof(float);
	const size_t is = tree.getIndexCapacity() * sizeof(unsigned);
	
	treeInfo.count[0] = tree.getVertexCount();
	treeInfo.count[1] = tree.getIndexCount();
	
	if (resized) {
		glBindBuffer(GL_ARRAY_BUFFER, bufferSets[1].buffers[0]);
		glBufferData(GL_ARRAY_BUFFER, vs, v, GL_DYNAMIC_DRAW);
		graphics::setVertexFormat(graphics::VERTEX_NORMAL);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferSets[1].buffers[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, is, i, GL_DYNAMIC_DRAW);
	} else {
		glBindBuffer(GL_ARRAY_BUFFER, bufferSets[1].buffers[0]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, vs, v);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferSets[1].buffers[1]);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, is, i);
	}

	updateSelection();
	update();
}

void Editor::changeResolution(int i)
{
	tree.setResolution(selectedStem, i);
	change();
}

void Editor::changeSections(int i)
{
	tree.setGeneratedPathSize(selectedStem, i);
	change();
}

void Editor::changeRadius(double d)
{
	tree.setRadius(selectedStem, d);
	change();
}

void Editor::changeRadiusCurve(std::vector<Vec3> c)
{
	tree.setRadiusCurve(selectedStem, &c[0], c.size());
	change();
}

void Editor::changeStemDensity(double d)
{
	tree.setStemDensity(selectedStem, d);
	change();
}

void Editor::changeBaseLength(double d)
{
	tree.setBaseLength(selectedStem, d);
	change();
}
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
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtOpenGL/QGLFormat>

#define UNUSED(x) (void)(x)

Editor::Editor(SharedResources *shared, QWidget *parent) : QOpenGLWidget(parent)
{
	this->shared = shared;
	ctrl = shift = midButton = false;
	selectedBranch = -1;
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

	shared->create();
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
	Geometry geometry;
	gridInfo = geometry.addGrid(5, {0.41, 0.41, 0.41}, {0.46, 0.46, 0.46});
	glGenVertexArrays(1, &bufferSets[0].vao);
	glBindVertexArray(bufferSets[0].vao);
	glGenBuffers(1, bufferSets[0].buffers);
	glBindBuffer(GL_ARRAY_BUFFER, bufferSets[0].buffers[0]);
	graphics::load(GL_ARRAY_BUFFER, geometry.vertices, GL_STATIC_DRAW);
	graphics::setVertexFormat(geometry.getVertexFormat());
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
			emit selectionChanged(tree, branch);
			return;
		}
		box = tmGetBoundingBox(tree, ++branch);
	}

	selectedBranch = -1;
	emit selectionChanged(tree, selectedBranch);
}

void Editor::mousePressEvent(QMouseEvent *event)
{
	QPoint p = event->pos();
	midButton = false;

	if (event->button() == Qt::RightButton) {
		selectBranch(p.x(), p.y());
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
	TMmat4 vp = camera.getVP();
	TMvec3 cp = camera.getPosition();

	glClearColor(0.5f, 0.5f, 0.5f, 1.0);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shared->getProgramName(shared->FLAT_SHADER));
	glBindVertexArray(bufferSets[0].vao);
	glUniformMatrix4fv(0, 1, GL_FALSE, &vp.m[0][0]);
	glDrawArrays(gridInfo.type, gridInfo.start[0], gridInfo.count[0]);

	glUseProgram(shared->getProgramName(shared->MODEL_SHADER));
	glBindVertexArray(bufferSets[1].vao);
	glUniformMatrix4fv(0, 1, GL_FALSE, &vp.m[0][0]);
	glUniform4f(1, cp.x, cp.y, cp.z, 0.0f);
	glDrawElements(GL_TRIANGLES, treeInfo.count[1], GL_UNSIGNED_SHORT, 0);

	if (selectedBranch != -1)
		paintSelection();

	glFlush();
}

/* This method assumes that the proper vertex array is already set. */
void Editor::paintSelection()
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

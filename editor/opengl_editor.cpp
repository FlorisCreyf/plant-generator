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

#include "opengl_editor.h"
#include "geometry.h"
#include "math.h"
#include <QtOpenGL/QGLFormat>

using treemaker::Vec3;
using treemaker::Mat4;

OpenGLEditor::OpenGLEditor(SharedResources *shared, QWidget *parent) :
		QOpenGLWidget(parent)
{
	this->shared = shared;
	setFocus();
}

void OpenGLEditor::initializeGL()
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
	configureTree();
	initializeGrid();
	intializeLines();
}

void OpenGLEditor::resizeGL(int width, int height)
{
	float ratio = static_cast<float>(width) / static_cast<float>(height);
	camera.setWindowSize(width, height);
	camera.setPerspective(45.0f, 0.1f, 100.0f, ratio);
	axis.setScale(height);
	glViewport(0, 0, width, height);
	paintGL();
}

void OpenGLEditor::initializeGrid()
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

void OpenGLEditor::initializeTree()
{
	glGenVertexArrays(1, &bufferSets[1].vao);
	glBindVertexArray(bufferSets[1].vao);
	glGenBuffers(2, bufferSets[1].buffers);

	treeInfo.type = GL_TRIANGLES;
	treeInfo.start[0] = treeInfo.start[1] = 0;
	treeInfo.count[0] = tree.getVertexCount();
	treeInfo.count[1] = tree.getIndexCount();

	change();
}

void OpenGLEditor::configureTree()
{
	tree.generateTree();
}

void OpenGLEditor::intializeLines()
{
	GLsizeiptr size = sizeof(Vec3) * maxLines;
	glGenVertexArrays(1, &bufferSets[2].vao);
	glBindVertexArray(bufferSets[2].vao);
	glGenBuffers(1, bufferSets[2].buffers);
	glBindBuffer(GL_ARRAY_BUFFER, bufferSets[2].buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	graphics::setVertexFormat(graphics::VERTEX_COLOR);
}

void OpenGLEditor::paintGL()
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

void OpenGLEditor::paintSelectionWireframe()
{
	Mat4 vp = camera.getVP();
	GLvoid *p = (GLvoid *)(selection.start[1] * sizeof(unsigned));

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

void OpenGLEditor::paintSelectionLines()
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

void OpenGLEditor::paintAxis()
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

void OpenGLEditor::updateLines(int stem)
{
	Geometry geom;
	const size_t size = tree.getPathSize(stem);
	std::vector<Vec3> points(size);
	tree.getPath(stem, &points[0]);
	lineInfo = geom.addLine(points, {0.76, 0.54, 0.29});

	glBindBuffer(GL_ARRAY_BUFFER, bufferSets[2].buffers[0]);
	if (points.size() > maxLines) {
		maxLines *= 2;
		GLsizeiptr size = sizeof(Vec3) * maxLines;
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
		graphics::setVertexFormat(geom.getVertexFormat());
	} else if (maxLines > minLines && points.size()/2 < maxLines) {
		maxLines /= 2;
		GLsizeiptr size = sizeof(Vec3) * maxLines;
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
		graphics::setVertexFormat(geom.getVertexFormat());
	}

	graphics::update(GL_ARRAY_BUFFER, 0, geom.vertices);
}

void OpenGLEditor::updateSelection()
{
	if (selectedStem != -1) {
		auto location = tree.getStemLocation(selectedStem);
		selection.start[1] = location.indexStart;
		selection.count[1] = location.indexCount;
	}
}

void OpenGLEditor::change()
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

/* Plant Generator
 * Copyright (C) 2019  Floris Creyf
 *
 * Plant Generator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Plant Generator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mesh_viewer.h"

using pg::Mat4;
using pg::Vec3;
using pg::DVertex;

MeshViewer::MeshViewer(SharedResources *shared, QWidget *parent) :
	QOpenGLWidget(parent)
{
	this->shared = shared;

	setFocusPolicy(Qt::StrongFocus);
	setMouseTracking(true);

	camera.setTarget(Vec3(0.0f, 0.0f, 0.0f));
	camera.setOrientation(0.0f, 90.0f);
	camera.setDistance(2.0f);
	camera.setPanSpeed(0.004f);
	camera.setZoom(0.01f, 0.3f, 10.0f);
}

QSize MeshViewer::sizeHint() const
{
	return QSize(width(), 200);
}

void MeshViewer::initializeGL()
{
	initializeOpenGLFunctions();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_LINE_SMOOTH);
	glDepthMask(GL_TRUE);
	glDepthRange(0.0f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_PRIMITIVE_RESTART);
	buffer.initialize(GL_DYNAMIC_DRAW);
	buffer.allocatePointMemory(100);
	buffer.allocateIndexMemory(100);
	glPrimitiveRestartIndex(Geometry::primitiveReset);
	shared->initialize();
	updateBuffer();
}

void MeshViewer::paintGL()
{
	glClearColor(0.22f, 0.22f, 0.22f, 1.0f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Mat4 vp = camera.updateVP();
	Vec3 cp = camera.getPosition();
	buffer.use();
	glUseProgram(shared->getShader(SharedResources::Solid));
	glUniformMatrix4fv(0, 1, GL_FALSE, &vp[0][0]);
	glUniform4f(1, cp.x, cp.y, cp.z, 0.0f);
	glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
	glFlush();
}

void MeshViewer::resizeGL(int width, int height)
{
	float ratio = static_cast<float>(width) / static_cast<float>(height);
	camera.setWindowSize(width, height);
	camera.setPerspective(45.0f, 0.1f, 200.0f, ratio);
}

void MeshViewer::mousePressEvent(QMouseEvent *event)
{
	QPoint p = event->pos();
	if (event->button() == Qt::MidButton) {
		camera.setStartCoordinates(p.x(), p.y());
		if (event->modifiers() & Qt::ControlModifier)
			camera.setAction(Camera::Zoom);
		else if (!(event->modifiers() & Qt::ShiftModifier))
			camera.setAction(Camera::Rotate);
	}
}

void MeshViewer::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::MidButton)
		camera.setAction(Camera::None);
}

void MeshViewer::mouseMoveEvent(QMouseEvent *event)
{
	QPoint pos = event->pos();
	camera.executeAction(pos.x(), pos.y());
	update();
}

void MeshViewer::wheelEvent(QWheelEvent *event)
{
	QPoint angleDelta = event->angleDelta();
	if (!angleDelta.isNull()) {
		float y = angleDelta.y() / 10.0f;
		if (y != 0.0f) {
			camera.zoom(y);
			update();
		}
	}
	event->accept();
}

void MeshViewer::updateMesh(pg::Geometry mesh)
{
	this->mesh = mesh;
	this->mesh.toCenter();
	if (isValid()) {
		makeCurrent();
		updateBuffer();
		doneCurrent();
	}
}

void MeshViewer::updateBuffer()
{
	std::vector<DVertex> points;
	for (DVertex vertex : mesh.getPoints())
		points.push_back(vertex);
	unsigned indexCount = mesh.getIndices().size();
	buffer.update(
		points.data(), points.size(),
		mesh.getIndices().data(), indexCount);
	count = indexCount;
	update();
}

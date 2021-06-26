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

const float pi = 3.14159265359f;

MeshViewer::MeshViewer(SharedResources *shared, QWidget *parent) :
	QOpenGLWidget(parent), shared(shared)
{
	setFocusPolicy(Qt::StrongFocus);
	setMouseTracking(true);

	this->camera.setTarget(Vec3(0.0f, 0.0f, 0.0f));
	this->camera.setOrientation(0.0f, 0.0f);
	this->camera.setDistance(2.0f);
	this->camera.setPanSpeed(0.004f);
	this->camera.setZoom(0.01f, 0.3f, 10.0f);
}

QSize MeshViewer::sizeHint() const
{
	return QSize(width(), 200);
}

void MeshViewer::initializeGL()
{
	initializeOpenGLFunctions();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);
	glDepthMask(GL_TRUE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_PRIMITIVE_RESTART);
	this->buffer.initialize(GL_DYNAMIC_DRAW);
	this->buffer.allocatePointMemory(100);
	this->buffer.allocateIndexMemory(100);
	glPrimitiveRestartIndex(Geometry::primitiveReset);
	this->shared->initialize();
	updateBuffer();
}

void MeshViewer::paintGL()
{
	glDepthFunc(GL_GEQUAL);
	glClearDepth(0.0f);
	glClearColor(0.22f, 0.22f, 0.22f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Mat4 vp = this->camera.getVP();
	Vec3 cp = this->camera.getPosition();
	this->buffer.use();
	glUseProgram(this->shared->getShader(SharedResources::Solid));
	glUniformMatrix4fv(0, 1, GL_FALSE, &vp[0][0]);
	glUniform3f(1, cp.x, cp.y, cp.z);
	glDrawElements(GL_TRIANGLES, this->count, GL_UNSIGNED_INT, 0);
	glFlush();
}

void MeshViewer::resizeGL(int width, int height)
{
	float ratio = static_cast<float>(width) / static_cast<float>(height);
	this->camera.setWindowSize(width, height);
	this->camera.setPerspective(pi/6.0f, -0.1f, -100.0f, ratio);
}

void MeshViewer::mousePressEvent(QMouseEvent *event)
{
	QPoint p = event->pos();
	if (event->button() == Qt::MidButton) {
		this->camera.setStartCoordinates(p.x(), p.y());
		if (event->modifiers() & Qt::ControlModifier)
			this->camera.setAction(Camera::Zoom);
		else if (!(event->modifiers() & Qt::ShiftModifier))
			this->camera.setAction(Camera::Rotate);
	}
}

void MeshViewer::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::MidButton)
		this->camera.setAction(Camera::None);
}

void MeshViewer::mouseMoveEvent(QMouseEvent *event)
{
	QPoint pos = event->pos();
	this->camera.executeAction(pos.x(), pos.y());
	update();
}

void MeshViewer::wheelEvent(QWheelEvent *event)
{
	QPoint angleDelta = event->angleDelta();
	if (!angleDelta.isNull()) {
		float y = angleDelta.y() / 10.0f;
		if (y != 0.0f) {
			this->camera.zoom(y);
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
	this->buffer.update(points.data(), points.size(),
		mesh.getIndices().data(), indexCount);
	this->count = indexCount;
	update();
}

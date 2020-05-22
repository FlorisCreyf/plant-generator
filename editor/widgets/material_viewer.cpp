/* Plant Genererator
 * Copyright (C) 2019  Floris Creyf
 *
 * Plant Genererator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Plant Genererator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "material_viewer.h"
#include "plant_generator/math/math.h"

using pg::Vec3;
using pg::Mat4;

MaterialViewer::MaterialViewer(SharedResources *shared, QWidget *parent) :
	QOpenGLWidget(parent)
{
	this->shared = shared;

	setFocusPolicy(Qt::StrongFocus);
	setMouseTracking(true);

	camera.setTarget(Vec3(0.5f, 0.0f, 0.5f));
	camera.setOrientation(180.0f, -180.0f);
	camera.setDistance(0.8f);
	camera.setPanSpeed(0.004f);
	camera.setZoom(0.01f, 0.3f, 2.0f);
}

QSize MaterialViewer::sizeHint() const
{
	return QSize(width(), 200);
}

void MaterialViewer::updateMaterial(ShaderParams params)
{
	this->params = params;
	update();
}

void MaterialViewer::createInterface()
{
	Geometry geometry;

	{
		Geometry plane;
		Vec3 a(1.0f, 0.0f, 0.0f);
		Vec3 b(0.0f, 0.0f, 1.0f);
		Vec3 center(0.0f, 0.2f, 0.0f);
		Vec3 color(0.34f, 0.34f, 0.34f);
		plane.addPlane(a, b, center, color);
		planeSegment = geometry.append(plane);
	}

	buffer.update(geometry);
}

void MaterialViewer::initializeGL()
{
	initializeOpenGLFunctions();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_ALWAYS);
	glDepthRange(0.0f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_PRIMITIVE_RESTART);
	buffer.initialize(GL_DYNAMIC_DRAW);
	buffer.allocatePointMemory(20);
	buffer.allocateIndexMemory(10);
	glPrimitiveRestartIndex(Geometry::primitiveReset);
	createInterface();
	shared->initialize();
	emit ready();
}

void MaterialViewer::paintGL()
{
	glClearColor(0.22f, 0.22f, 0.22f, 1.0f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	camera.updateVP();
	float aspect = params.getMaterial().getRatio();
	Mat4 scale;
	if (aspect < 1.0f)
		scale = pg::scale(Vec3(1.0f, 1.0f/aspect, 1.0f));
	else
		scale = pg::scale(Vec3(aspect, 1.0f, 1.0f));
	Mat4 vp = scale * camera.getVP();
	buffer.use();
	glUseProgram(shared->getShader(SharedResources::Material));
	glUniformMatrix4fv(0, 1, GL_FALSE, &vp[0][0]);

	GLuint texture = params.getTexture(0);
	if (texture == 0)
		texture = shared->getTexture(SharedResources::DefaultTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	auto size = sizeof(unsigned);
	GLvoid *start = (GLvoid *)(planeSegment.istart * size);
	GLsizei count = planeSegment.icount;
	glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, start);

	glFlush();
}

void MaterialViewer::resizeGL(int width, int height)
{
	float ratio = static_cast<float>(width) / static_cast<float>(height);
	camera.setWindowSize(width, height);
	camera.setOrthographic(
		Vec3(-ratio, -1.0f, 0.0f), Vec3(ratio, 1.0f, 100.0f));
}

void MaterialViewer::mousePressEvent(QMouseEvent *event)
{
	QPoint p = event->pos();
	if (event->button() == Qt::MidButton) {
		camera.setStartCoordinates(p.x(), p.y());
		if (event->modifiers() & Qt::ControlModifier)
			camera.setAction(Camera::Zoom);
		else if (event->modifiers() & Qt::ShiftModifier)
			camera.setAction(Camera::Pan);
	}
}

void MaterialViewer::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::MidButton)
		camera.setAction(Camera::None);
}

void MaterialViewer::mouseMoveEvent(QMouseEvent *event)
{
	QPoint p = event->pos();
	camera.executeAction(p.x(), p.y());
	update();
}

void MaterialViewer::wheelEvent(QWheelEvent *event)
{
	QPoint a = event->angleDelta();
	if (!a.isNull()) {
		float y = a.y() / 10.0f;
		if (y != 0.0f) {
			camera.zoom(y);
			update();
		}
	}
	event->accept();
}

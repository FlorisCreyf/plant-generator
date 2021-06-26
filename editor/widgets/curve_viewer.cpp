/* Plant Generator
 * Copyright (C) 2020  Floris Creyf
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

#include "curve_viewer.h"

#ifdef _WIN32
#undef near
#undef far
#endif

using pg::Mat4;
using pg::Vec3;
using pg::Spline;

CurveViewer::CurveViewer(SharedResources *shared, QWidget *parent) :
	QOpenGLWidget(parent), shared(shared)
{
	setFocusPolicy(Qt::StrongFocus);
	setMouseTracking(true);

	QSizePolicy policy;
	policy.setHorizontalPolicy(QSizePolicy::Preferred);
	policy.setVerticalPolicy(QSizePolicy::Preferred);
	policy.setVerticalStretch(1);
	policy.setHorizontalStretch(1);
	this->setSizePolicy(policy);

	this->camera.setTarget(Vec3(0.5f, 0.5f, 0.0f));
	this->camera.setOrientation(0.0f, 0.0f);
	this->camera.setDistance(0.7f);
	this->camera.setPanSpeed(0.004f);
	this->camera.setZoom(0.01f, 0.3f, 2.0f);

	this->path.setColor(
		Vec3(0.5f, 0.5f, 0.5f),
		Vec3(0.6f, 0.6f, 0.6f),
		Vec3(0.1f, 1.0f, 0.4f));
	appendInterface(this->geometry);
}

const Camera *CurveViewer::getCamera() const
{
	return &this->camera;
}

void CurveViewer::initializeGL()
{
	initializeOpenGLFunctions();
	glPointSize(8);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_ALWAYS);
	glDepthRange(0.0f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(Geometry::primitiveReset);
	this->buffer.initialize(GL_DYNAMIC_DRAW);
	this->buffer.allocatePointMemory(100);
	this->buffer.allocateIndexMemory(100);
	this->buffer.update(geometry);
	shared->initialize();
	update();
}

void CurveViewer::appendInterface(Geometry &geometry)
{
	Geometry plane;
	Vec3 a(1.0f, 0.0f, 0.0f);
	Vec3 b(0.0f, 1.0f, 0.0f);
	Vec3 center(0.0f, 0.0f, 0.0f);
	Vec3 color(0.34f, 0.34f, 0.34f);
	plane.addPlane(a, b, center, color, Vec3(0.0f, 0.0f, 0.0f));
	this->plane = geometry.append(plane);

	Geometry grid;
	Vec3 colors[2];
	colors[0] = Vec3(0.3f, 0.3f, 0.3f);
	colors[1] = Vec3(0.3f, 0.3f, 0.3f);
	Mat4 t(
		1.0f/6.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f/6.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f/6.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);
	grid.addGrid(100, colors, colors[0]);
	Geometry::Segment segment = grid.getSegment();
	grid.transform(segment.pstart, segment.pcount, t);
	this->grid = geometry.append(grid);
}

void CurveViewer::change(const Spline &spline, const PointSelection &selection)
{
	int resolution = spline.getDegree() == 3 ? 20 : 1;
	this->path.set(spline, resolution, Vec3(0.0f, 0.0f, 0.0f));
	this->path.setSelectedPoints(selection);
	this->geometry = Geometry();
	this->geometry.append(*this->path.getGeometry());

	appendInterface(this->geometry);
	if (isValid()) {
		makeCurrent();
		this->buffer.update(this->geometry);
		doneCurrent();
		update();
	}
}

void CurveViewer::clear()
{
	this->path.clearPoints();
	this->geometry.clear();
	appendInterface(this->geometry);
	if (isValid()) {
		makeCurrent();
		this->buffer.update(this->geometry);
		doneCurrent();
		update();
	}
}

void CurveViewer::wheelEvent(QWheelEvent *event)
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

void CurveViewer::mousePressEvent(QMouseEvent *event)
{
	QPoint pos = event->pos();
	if (event->button() == Qt::MidButton) {
		this->camera.setStartCoordinates(pos.x(), pos.y());
		if (event->modifiers() & Qt::ControlModifier)
			this->camera.setAction(Camera::Zoom);
		else if (event->modifiers() & Qt::ShiftModifier)
			this->camera.setAction(Camera::Pan);
	}
}

void CurveViewer::mouseMoveEvent(QMouseEvent *event)
{
	QPoint pos = event->pos();
	if (this->camera.executeAction(pos.x(), pos.y()))
		update();
}

void CurveViewer::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::MidButton)
		this->camera.setAction(Camera::None);
}

void CurveViewer::resizeGL(int width, int height)
{
	float ratio = 1.0f;
	Vec3 near(ratio, 1.0f, 0.0f);
	Vec3 far(-ratio, -1.0f, -100.0f);
	this->camera.setWindowSize(width, height);
	this->camera.setOrthographic(near, far);
}

void CurveViewer::paintGL()
{
	glViewport(0, 0, width(), height());
	glClearColor(0.32f, 0.32f, 0.32f, 1.0);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Mat4 vp = this->camera.getVP();

	this->buffer.use();
	glUseProgram(this->shared->getShader(SharedResources::Flat));
	glUniformMatrix4fv(0, 1, GL_FALSE, &vp[0][0]);

	{
		auto size = sizeof(unsigned);
		GLvoid *start = (GLvoid *)(this->plane.istart * size);
		GLsizei count = this->plane.icount;
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, start);
	}

	glDrawArrays(GL_LINES, this->grid.pstart, this->grid.pcount);

	if (this->path.getLineSegment().pcount) {
		glUseProgram(this->shared->getShader(SharedResources::Line));
		glUniformMatrix4fv(0, 1, GL_FALSE, &vp[0][0]);
		glUniform2f(1, QWidget::width(), QWidget::height());

		Geometry::Segment s = this->path.getLineSegment();
		GLvoid *start = (GLvoid *)(s.istart * sizeof(unsigned));
		glDrawElements(GL_LINE_STRIP, s.icount, GL_UNSIGNED_INT, start);

		s = this->path.getPointSegment();
		GLuint texture = this->shared->getTexture(
			SharedResources::DotTexture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUseProgram(this->shared->getShader(SharedResources::Point));
		glUniformMatrix4fv(0, 1, GL_FALSE, &vp[0][0]);
		glDrawArrays(GL_POINTS, s.pstart, s.pcount);
	}

	glFlush();
}

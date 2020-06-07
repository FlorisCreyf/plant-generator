/* Plant Generator
 * Copyright (C) 2016-2018  Floris Creyf
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

#include "curve_button.h"
#include "editor/geometry/geometry.h"

using pg::Vec3;
using pg::Mat4;

CurveButton::CurveButton(
	QString name, SharedResources *shared, QWidget *parent) :
	QOpenGLWidget(parent)
{
	setCursor(Qt::PointingHandCursor);
	this->name = name;
	this->shared = shared;
	this->enabled = true;
}

void CurveButton::setEnabled(bool enabled)
{
	this->enabled = enabled;
	QWidget::setEnabled(enabled);
	update();
}

void CurveButton::initializeGL()
{
	initializeOpenGLFunctions();
	buffer.initialize(GL_DYNAMIC_DRAW);
	buffer.allocatePointMemory(100);
	setCurve(spline);
}

void CurveButton::setCurve(const pg::Spline &spline)
{
	this->spline = spline;
	if (isValid()) {
		geometry.clear();
		geometry.addCurve(spline, Vec3(0.7f, 0.7f, 0.7f), 6);
		segment = geometry.getSegment();
		buffer.update(geometry);
		update();
	}
}

void CurveButton::paintGL()
{
	Mat4 vp(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, 0.0f, 1.0f);

	glClearColor(0.2666f, 0.2666f, 0.2666f, 1.0);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (enabled) {
		buffer.use();
		glUseProgram(shared->getShader(SharedResources::Flat));
		glUniformMatrix4fv(0, 1, GL_FALSE, &vp[0][0]);
		glDrawArrays(GL_LINE_STRIP, segment.pstart, segment.pcount);
	}

	glFlush();
}

QString CurveButton::getName()
{
	return name;
}

void CurveButton::select()
{
	emit selected(this);
}

void CurveButton::mousePressEvent(QMouseEvent *)
{
	select();
}

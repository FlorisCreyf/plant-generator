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

#include "curve_button.h"
#include "geometry.h"

using treemaker::Vec3;
using treemaker::Mat4;

CurveButton::CurveButton(QString name, SharedResources *shared, QWidget *parent)
		: QOpenGLWidget(parent)
{
	setCursor(Qt::PointingHandCursor);
	this->name = name;
	this->shared = shared;
}

void CurveButton::initializeGL()
{
	initializeOpenGLFunctions();
	glViewport(0, 0, 26, 18);
	glGenVertexArrays(1, &bufferSet.vao);
	glBindVertexArray(bufferSet.vao);
	glGenBuffers(1, bufferSet.buffers);
	createGeometry();
	update();
}

void CurveButton::createGeometry()
{
	Geometry geom;
	curveInfo = geom.addBPath(controls, 10, {0.6f, 0.6f, 0.6f});
	glBindBuffer(GL_ARRAY_BUFFER, bufferSet.buffers[0]);
	graphics::load(GL_ARRAY_BUFFER, geom.vertices, GL_DYNAMIC_DRAW);
	graphics::setVertexFormat(geom.getVertexFormat());
}

QSize CurveButton::sizeHint() const
{
	return QSize(26, 18);
}

void CurveButton::setControls(Vec3 *controls, int size)
{
	std::vector<Vec3> v;
	v.insert(v.begin(), controls, controls+size);
	setControls(v);
}

void CurveButton::setControls(std::vector<Vec3> controls)
{
	this->controls = controls;
	if (isValid()) {
		createGeometry();
		update();
	}
}

std::vector<Vec3> CurveButton::getControls()
{
	return controls;
}

void CurveButton::paintGL()
{
	Mat4 vp = {
		1.8f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.9f, 0.0f,
		0.0f, 1.8f, 0.0f, 0.0f,
		-0.9f, -0.9f, 0.0f, 1.0f
	};

	glClearColor(0.32f, 0.32f, 0.32f, 1.0);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shared->getProgramName(shared->FLAT_SHADER));
	glBindVertexArray(bufferSet.vao);
	glUniformMatrix4fv(0, 1, GL_FALSE, &vp[0][0]);
	glDrawArrays(curveInfo.type, curveInfo.start[0], curveInfo.count[0]);

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

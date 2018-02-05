/* Plant Genererator
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

#include "curve_editor.h"
#include "../geometry/geometry.h"
#include "plant_generator/math/curve.h"
#include "plant_generator/math/intersection.h"
#include <QtGui/QMouseEvent>
#include <QTabBar>
#include <cmath>
#include <limits>

using pg::Vec3;
using pg::Mat4;

CurveEditor::CurveEditor(SharedResources *shared, QWidget *parent) :
	QOpenGLWidget(parent)
{
	this->shared = shared;
	point = std::numeric_limits<size_t>::max();
	enabled = false;

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setSizeConstraint(QLayout::SetMinimumSize);
	layout->setSpacing(0);
	layout->setMargin(0);

	degree = new QComboBox;
	degree->addItem(QString("Linear"));
	degree->addItem(QString("Cubic"));
	degree->setFixedHeight(22);
	degree->setHidden(true);
	layout->addWidget(degree);
	layout->addStretch(1);

	connect(degree, SIGNAL(currentIndexChanged(int)), this,
		SLOT(setDegree(int)));
}

QSize CurveEditor::sizeHint() const
{
	return QSize(300, 40);
}

void CurveEditor::initializeGL()
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

	buffer.initialize(GL_DYNAMIC_DRAW);
	buffer.allocatePointMemory(1000);
	buffer.allocateIndexMemory(1000);

	path.setColor({0.5f, 0.5f, 0.5f}, {0.6f, 0.6f, 0.6f});
	glPrimitiveRestartIndex(Geometry::primitiveReset);

	createInterface();
}

void CurveEditor::createInterface()
{
	Geometry geometry;

	{
		path.set(spline, 20);
		geometry.append(*path.getGeometry());
	}

	{
		Geometry plane;
		Vec3 a = {1.0f, 0.0f, 0.0f};
		Vec3 b = {0.0f, 0.0f, 1.0f};
		Vec3 center = {0.0f, 0.2f, 0.0f};
		Vec3 color = {0.33f, 0.335f, 0.34f};
		plane.addPlane(a, b, center, color);
		planeSegment = geometry.append(plane);
	}

	{
		Geometry grid;
		Vec3 color = {0.3f, 0.3f, 0.3f};
		Mat4 t = {
			1.0f/4.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f/4.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f/4.0f, 0.0f,
			0.5f, 0.0f, 0.5f, 1.0f
		};
		grid.addGrid(3, color, color);
		Geometry::Segment segment = grid.getSegment();
		grid.transform(segment.pstart, segment.pcount, t);
		gridSegment = geometry.append(grid);
	}

	buffer.update(geometry);
}

void CurveEditor::resizeGL(int width, int height)
{
	glViewport(0, 0, width, height);
	this->width = width;
	this->height = height;
}

void CurveEditor::mousePressEvent(QMouseEvent *event)
{
	std::vector<pg::Vec3> controls = spline.getControls();
	if (!enabled || event->button() != Qt::LeftButton)
		return;

	QPoint p = event->pos();
	for (size_t i = 0; i < controls.size(); i++) {
		int w = width - margin;
		int h = height - margin;
		int x = (controls[i].x) * w + margin/2;
		int y = (h - (controls[i].z) * (h - topMargin)) + margin/2;
		float dx2 = std::pow(p.x() - x, 2);
		float dy2 = std::pow(p.y() - y, 2);

		if (std::sqrt(dx2 + dy2) < 8) {
			point = i;
			insertIndex = std::numeric_limits<size_t>::max();
			x = p.x();
			y = p.y();
			setTangentLength();
		}
	}
}

void CurveEditor::toDeviceCoordinates(float &x, float &y, int width, int height)
{
	x = (x - 10) / (width - 1 - margin);
	y = (1.0f - (y - 10 - topMargin) / (height - 1 - margin - topMargin));
}

void CurveEditor::insertPoint(int i, float x, float y)
{
	std::vector<pg::Vec3> controls = spline.getControls();
	if (x <= controls[i-1].x || x >= controls[i].x)
		return;
	if (y <= 0.0f || y >= 1.0f)
		return;
	spline.insert(i - 1, {x, -0.3f, y});
	createInterface();
	emit curveChanged(spline, name);
	update();
}

void CurveEditor::insertCurve(int i, float x, float y)
{
	std::vector<pg::Vec3> controls = spline.getControls();
	float len = 0.2f;
	float minX = x - len > controls[i-3].x ? x - len : controls[i-3].x;
	float maxX = x + len < controls[i].x ? x + len : controls[i].x;

	if (x <= controls[i].x && x >= controls[i-1].x)
		return;
	if (x >= controls[i-3].x && x <= controls[i-2].x)
		return;
	if (y <= 0.0f || y >= 1.0f)
		return;

	pg::Vec3 curve[4];
	curve[0] = {minX, -0.3f, y};
	curve[1] = {x, -0.3f, y};
	curve[2] = {maxX, -0.3f, y};
	controls.insert(controls.begin()+i-1, curve, &curve[3]);

	spline.setControls(controls);
	createInterface();
	emit curveChanged(spline, name);
	update();
}

void CurveEditor::mouseDoubleClickEvent(QMouseEvent *event)
{
	if (enabled && event->button() == Qt::LeftButton) {
		QPoint p = event->pos();
		float x = p.x();
		float y = p.y();
		toDeviceCoordinates(x, y, width, height);

		std::vector<pg::Vec3> controls = spline.getControls();
		size_t increment = spline.getDegree() == 3 ? 3 : 1;
		for (size_t i = increment; i < controls.size(); i += increment)
			if (controls[i].x > x) {
				if (spline.getDegree() == 3)
					insertCurve(i, x, y);
				else
					insertPoint(i, x, y);
				break;
			}
	}
}

void CurveEditor::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
		point = std::numeric_limits<size_t>::max();
}

void CurveEditor::mouseMoveEvent(QMouseEvent *event)
{
	QPoint p = event->pos();
	float x = p.x();
	float y = p.y();
	const size_t max = std::numeric_limits<size_t>::max();
	bool removed = insertIndex != 0 && insertIndex != max;
	toDeviceCoordinates(x, y, width, height);

	if (point == std::numeric_limits<size_t>::max())
		return;
	if (removed && !reinsertCurve(x))
		return;

	if (spline.getDegree() == 1) {
		if (!omitCurve(x))
			moveControl(x, y);
	} else if (point == 0)
		moveTerminalControl(true, y);
	else if (point == spline.getControls().size() - 1)
		moveTerminalControl(false, y);
	else if (point % 3 == 0) {
		if (!omitCurve(x))
			moveOuterControl(x, y);
	} else
		moveInnerControl(x, y);

	createInterface();
	emit curveChanged(spline, name);
	update();
}

bool CurveEditor::reinsertCurve(float x)
{
	std::vector<pg::Vec3> controls = spline.getControls();

	if (spline.getDegree() == 1) {
		if (x > controls[point-1].x && x < controls[point].x) {
			auto index = controls.begin() + insertIndex;
			controls.insert(index, hidden[0]);
			createInterface();
			insertIndex = std::numeric_limits<size_t>::max();
			spline.setControls(controls);
			return true;
		}
	} else if (x > controls[point-2].x && x < controls[point].x) {
		auto index = controls.begin() + insertIndex;
		controls.insert(index, &hidden[0], &hidden[3]);
		createInterface();
		insertIndex = std::numeric_limits<size_t>::max();
		spline.setControls(controls);
		return true;
	}
	return false;
}

bool CurveEditor::omitCurve(float x)
{
	std::vector<pg::Vec3> controls = spline.getControls();
	int i = spline.getDegree() == 1 ? 1 : 3;

 	if (x < controls[point-i].x || x > controls[point+i].x) {
		insertIndex = spline.getDegree() == 3 ? point - 1 : point;
		auto start = controls.begin() + insertIndex;
		auto end = controls.begin() + insertIndex + i;
		memcpy(hidden, &controls[insertIndex], sizeof(Vec3)*i);
		controls.erase(start, end);
		spline.setControls(controls);
		return true;
	} else
		return false;
}

void CurveEditor::moveControl(float x, float y)
{
	std::vector<pg::Vec3> controls = spline.getControls();

	if (point == 0)
		x = 0.0f;
	else if (point == controls.size() - 1)
		x = 1.0f;
	else {
		if (x < controls[point-1].x)
			x = controls[point-1].x;
		else if (x > controls[point+1].x)
			x = controls[point+1].x;
	}

	if (y > 1.0f)
		y = 1.0f;
	else if (y < 0.0f)
		y = 0.0f;

	controls[point].x = x;
	controls[point].z = y;
	spline.setControls(controls);
}

void CurveEditor::moveOuterControl(float x, float y)
{
	std::vector<pg::Vec3> controls = spline.getControls();
	float dx1 = controls[point-1].x - controls[point].x;
	float dy1 = controls[point-1].z - controls[point].z;
	float dx2 = controls[point+1].x - controls[point].x;
	float dy2 = controls[point+1].z - controls[point].z;

	if (x < controls[point-2].x)
		x = controls[point-2].x;
	if (x > controls[point+2].x)
		x = controls[point+2].x;
	if (x + dx1 < controls[point-3].x)
		x = controls[point-3].x - dx1;
	if (x + dx2 > controls[point+3].x)
		x = controls[point+3].x - dx2;
	if (y + dy1 > 1.0f)
		y = 1.0f - dy1;
	if (y + dy1 < 0.0f)
		y = -dy1;
	if (y + dy2 < 0.0f)
		y = -dy2;
	if (y + dy2 > 1.0f)
		y = 1.0f - dy2;
	if (y > 1.0f)
		y = 1.0f;
	if (y < 0.0f)
		y = 0.0f;

	controls[point+1].x = x + dx2;
	controls[point+1].z = y + dy2;
	controls[point+0].x = x;
	controls[point+0].z = y;
	controls[point-1].x = x + dx1;
	controls[point-1].z = y + dy1;
	spline.setControls(controls);
}

void CurveEditor::moveInnerControl(float x, float y)
{
	int l = point % 3 == 1 ? 1 : 0;

	spline.move(point, {x, -0.3f, y});
	std::vector<pg::Vec3> controls = spline.getControls();

	if (controls[point].x < controls[point-2+l].x)
		controls[point].x = controls[point-2+l].x;
	else if (controls[point].x > controls[point+1+l].x)
		controls[point].x = controls[point+1+l].x;

	if (controls[point].z > 1.0f)
		controls[point].z = 1.0f;
	else if (controls[point].z < 0.0f)
		controls[point].z = 0.0f;

	spline.setControls(controls);
	moveOppositeTangent();
}

/** Here we assume that tangentLength is set. */
void CurveEditor::moveOppositeTangent()
{
	std::vector<pg::Vec3> controls = spline.getControls();
	if (point != 1 && point != controls.size() - 2) {
		int d = point % 3 == 1 ? -1 : 1;

		pg::Ray2 tangent;
		pg::Ray2 boundary;
		int i = point+2*d;
		int j = point+1*d;
		tangent.origin = toVec2(controls[j]);
		tangent.direction = toVec2(normalize(controls[i] -
			controls[j]));

		float t;
		float r;
		boundary.origin = (pg::Vec2){0.0f, 1.0f};
		boundary.direction = (pg::Vec2){1.0f, 0.0f};
		t = intersectsLine(tangent, boundary);
		boundary.origin = (pg::Vec2){0.0f, 0.0f};
		boundary.direction = (pg::Vec2){1.0f, 0.0f};
		r = intersectsLine(tangent, boundary);
		if (t < 0 || (r < t && r >= 0))
			t = r;
		boundary.origin = (pg::Vec2){controls[point+4*d].x, 0.0f};
		boundary.direction = (pg::Vec2){0.0f, 1.0f};
		r = intersectsLine(tangent, boundary);
		if (t < 0 || (r < t && r >= 0))
			t = r;

		if (t < 0 || tangentLength < t)
			t = tangentLength;

		pg::Vec2 p = t * normalize(tangent.direction) + tangent.origin;
		controls[i].x = p.x;
		controls[i].z = p.y;
		spline.setControls(controls);
	}
}

void CurveEditor::setTangentLength()
{
	std::vector<pg::Vec3> controls = spline.getControls();
	int r = point % 3;
	if (r != 0) {
		r = r == 1 ? -1 : 1;
		int a = point + 2 * r;
		int b = point + 1 * r;
		pg::Vec3 v = controls[a] - controls[b];
		tangentLength = magnitude(v);
	}
}

void CurveEditor::moveTerminalControl(bool first, float y)
{
	std::vector<pg::Vec3> controls = spline.getControls();
	int adjacent = first ? 1 : -1;
	float dy = controls[point + adjacent].z - controls[point].z;

	if (y + dy > 1.0f)
		y = 1.0f - dy;
	else if (y + dy < 0.0f)
		y = -dy;
	if (y > 1.0f)
		y = 1.0f;
	else if (y < 0.0f)
		y = 0.0f;

	controls[point + adjacent].z = y + dy;
	controls[point].z = y;
	spline.setControls(controls);
}

pg::Mat4 CurveEditor::createVP()
{
	float w = QWidget::width();
	float h = QWidget::height();
	float marginWRatio = margin / w;
	float marginHRatio = margin / h;
	float planeWRatio = (w - margin) / w;
	float planeHRatio = (h - (margin + topMargin)) / h;

	return (Mat4){
		2.0f * planeWRatio, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.9f, 0.0f,
		0.0f, 2.0f * planeHRatio, 0.0f, 0.0f,
		-1.0f + marginWRatio, -1.0f + marginHRatio, 0.0f, 1.0f
	};
}

void CurveEditor::paintGL()
{
	glClearColor(0.32f, 0.32f, 0.32f, 1.0);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (QWidget::height() > margin * 2)
	 	paintInterface();

	glFlush();
}

void CurveEditor::paintInterface()
{
	Mat4 vp = createVP();
	buffer.use();
	glUseProgram(shared->getShader(Shader::Flat));
	glUniformMatrix4fv(0, 1, GL_FALSE, &vp[0][0]);

	{
		GLvoid *start = (GLvoid *)(planeSegment.istart *
			sizeof(unsigned));
		GLsizei count = planeSegment.icount;
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, start);
	}

	glDrawArrays(GL_LINES, gridSegment.pstart, gridSegment.pcount);

	if (enabled) {
		Geometry::Segment s = path.getLineSegment();
		GLvoid *start = (GLvoid *)(s.istart * sizeof(unsigned));
		glDrawElements(GL_LINE_STRIP, s.icount, GL_UNSIGNED_INT, start);

		s = path.getPointSegment();
		GLuint texture = shared->getTexture(shared->DotTexture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUseProgram(shared->getShader(Shader::Point));
		glUniformMatrix4fv(0, 1, GL_FALSE, &vp[0][0]);
		glDrawArrays(GL_POINTS, s.pstart, s.pcount);
	}
}

void CurveEditor::setCurve(pg::Spline spline, QString name)
{
	parentWidget()->setWindowTitle(name + " Curve");

	degree->blockSignals(true);
	switch(spline.getDegree()) {
	case 1:
		degree->setCurrentIndex(0);
		break;
	case 3:
		degree->setCurrentIndex(1);
		break;
	}
	degree->setHidden(false);
	degree->blockSignals(false);

	this->spline = spline;
	this->name = name;
	createInterface();
	update();
}

void CurveEditor::setDegree(int degree)
{
	degree = degree == 1 ? 3 : 1;
	if (degree != spline.getDegree()) {
		spline.adjust(degree);
		if (spline.getDegree() == 3) {
			std::vector<pg::Vec3> controls = spline.getControls();
			for (size_t i = 2; i < controls.size() - 2; i += 3) {
				point = i;
				setTangentLength();
				moveOppositeTangent();
				point += 2;
				moveOppositeTangent();
			}
			point = std::numeric_limits<size_t>::max();
		}

		createInterface();
		update();
		emit curveChanged(spline, name);
	}
}

void CurveEditor::setEnabled(bool enabled)
{
	if (enabled && name.size() > 0) {
		degree->setHidden(false);
		parentWidget()->setWindowTitle(name + " Curve");
	} else {
		degree->setHidden(true);
		parentWidget()->setWindowTitle("Curve");
	}

	this->enabled = enabled;
	update();
}

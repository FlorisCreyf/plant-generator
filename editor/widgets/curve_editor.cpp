/* Plant Genererator
 * Copyright (C) 2016-2018  Floris Creyf
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

#include "curve_editor.h"
#include "../commands/extrude_spline.h"
#include "../commands/remove_spline.h"
#include "../commands/save_point_selection.h"
#include "../geometry/geometry.h"
#include "plant_generator/math/curve.h"
#include "plant_generator/math/intersection.h"
#include <QtGui/QMouseEvent>
#include <QTabBar>
#include <cmath>

using pg::Vec3;
using pg::Mat4;

CurveEditor::CurveEditor(SharedResources *shared, QWidget *parent) :
	QOpenGLWidget(parent),
	selection(&camera),
	moveSpline(&selection, &origSpline, &axes)
{
	this->shared = shared;
	enabled = false;

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setSizeConstraint(QLayout::SetMinimumSize);
	layout->setSpacing(0);
	layout->setMargin(0);
	degree = new QComboBox(this);
	degree->addItem(QString("Linear"));
	degree->addItem(QString("Cubic"));
	degree->setFixedHeight(22);
	degree->setEnabled(false);
	layout->addWidget(degree);
	layout->addStretch(1);
	setFocusPolicy(Qt::StrongFocus);
	setMouseTracking(true);

	camera.setTarget({0.5f, 0.0f, 0.5f});
	camera.setOrientation(180.0f, -180.0f);
	camera.setDistance(1.0f);
	camera.setPanSpeed(0.004f);
	camera.setZoom(0.01f, 0.3f, 2.0f);

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

	path.setColor({0.5f, 0.5f, 0.5f}, {0.6f, 0.6f, 0.6f},
		{0.1f, 1.0f, 0.4f});
	glPrimitiveRestartIndex(Geometry::primitiveReset);

	createInterface();
}

void CurveEditor::focusOutEvent(QFocusEvent *event)
{
	(void)event;
	emit editingFinished();
}

void CurveEditor::createInterface()
{
	Geometry geometry;

	{
		path.set(spline, 20, {0.0f, 0.0f, 0.0f});
		path.setSelectedPoints(selection);
		geometry.append(*path.getGeometry());
	}

	{
		Geometry plane;
		Vec3 a = {1.0f, 0.0f, 0.0f};
		Vec3 b = {0.0f, 0.0f, 1.0f};
		Vec3 center = {0.0f, 0.2f, 0.0f};
		Vec3 color = {0.34f, 0.34f, 0.34f};
		plane.addPlane(a, b, center, color);
		planeSegment = geometry.append(plane);
	}

	{
		Geometry grid;
		Vec3 color = {0.3f, 0.3f, 0.3f};
		Mat4 t = {
			1.0f/6.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f/6.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f/6.0f, 0.0f,
			0.5f, 0.0f, 0.5f, 1.0f
		};
		grid.addGrid(20, color, color);
		Geometry::Segment segment = grid.getSegment();
		grid.transform(segment.pstart, segment.pcount, t);
		gridSegment = geometry.append(grid);
	}

	buffer.update(geometry);
}

void CurveEditor::resizeGL(int width, int height)
{
	float ratio = static_cast<float>(width) / static_cast<float>(height);
	camera.setWindowSize(width, height);
	camera.setOrthographic({-ratio, -1.0f, 0.0f}, {ratio, 1.0f, 100.0f});
	glViewport(0, 0, width, height);
}

void CurveEditor::keyPressEvent(QKeyEvent *event)
{
	switch (event->key()) {
	case Qt::Key_1:
		if (event->modifiers() & Qt::ControlModifier) {
			SavePointSelection selectionCopy(&selection);
			selection.selectNext(spline.getSize());
			if (selectionCopy.hasChanged()) {
				selectionCopy.setAfter();
				history.add(selectionCopy);
				createInterface();
				update();
			}
		}
		break;
	case Qt::Key_2:
		if (event->modifiers() & Qt::ControlModifier) {
			SavePointSelection selectionCopy(&selection);
			selection.selectPrevious();
			if (selectionCopy.hasChanged()) {
				selectionCopy.setAfter();
				history.add(selectionCopy);
				createInterface();
				update();
			}
		}
		break;
	case Qt::Key_3:
		if (event->modifiers() & Qt::ControlModifier) {
			SavePointSelection selectionCopy(&selection);
			if (selection.getPoints().empty())
				selection.selectAll(spline.getSize());
			else
				selection.clear();
			if (selectionCopy.hasChanged()) {
				selectionCopy.setAfter();
				history.add(selectionCopy);
				createInterface();
				update();
			}
		}
		break;
	case Qt::Key_E:
		extrude();
		break;
	case Qt::Key_Z:
		if (event->modifiers() & Qt::ControlModifier) {
			if (event->modifiers() & Qt::ShiftModifier)
				history.redo();
			else
				history.undo();
			spline = origSpline;
			applyRestrictions();
			createInterface();
			update();
			emit curveChanged(spline, name);
		}
		break;
	case Qt::Key_Delete:
		if (!move) {
			/* The first and last points cannot be removed. */
			auto points = selection.getPoints();
			int last = spline.getSize() - 1;
			points.erase(last);
			points.erase(0);
			if (spline.getDegree() == 3) {
				points.erase(last - 1);
				points.erase(1);
			}
			selection.setPoints(points);

			RemoveSpline cmd(&selection, &origSpline);
			cmd.execute();
			spline = origSpline;
			createInterface();
			update();
			history.add(cmd);
			emit curveChanged(spline, name);
		}
		break;
	default:
		break;
	}
}

void CurveEditor::extrude()
{
	auto points = selection.getPoints();
	int last = spline.getSize() - 1;

	/* The last point cannot be extruded. */
	points.erase(last);

	/* Extrusion should fail if tangents overlap. */
	if (spline.getDegree() == 3) {
		auto controls = spline.getControls();
		for (auto s : points) {
			if (s % 3 == 1 && controls[s+1].x < controls[s].x)
				points.erase(s);
			if (s % 3 == 0 && controls[s+2].x < controls[s+1].x)
				points.erase(s);
			if (s % 3 == 2 && controls[s-1].x > controls[s].x)
				points.erase(s);
		}
	}

	if (points.empty())
		return;

	selection.setPoints(points);

	QPoint p = mapFromGlobal(QCursor::pos());
	Vec3 avg = pg::getZeroVec3();
	avg = selection.getAveragePosition(spline, avg);
	axes.setPosition(avg);
	setClickOffset(p.x(), p.y(), avg);

	origSpline = spline;
	ExtrudeSpline cmd(&selection, &origSpline);
	cmd.execute();
	spline = origSpline;
	extruding = true;
	initiateMovePoint();
	applyRestrictions();
	createInterface();
	update();
	history.add(cmd);
	emit curveChanged(spline, name);
}

void CurveEditor::mousePressEvent(QMouseEvent *event)
{
	QPoint p = event->pos();
	move = false;

	if (event->button() == Qt::MidButton) {
		camera.setStartCoordinates(p.x(), p.y());
		if (event->modifiers() & Qt::ControlModifier)
			camera.setAction(Camera::Zoom);
		else if (event->modifiers() & Qt::ShiftModifier)
			camera.setAction(Camera::Pan);
	} else if (enabled && event->button() == Qt::RightButton) {
		SavePointSelection selectionCopy(&selection);
		Vec3 zero = pg::getZeroVec3();
		selection.selectPoint(event, spline, zero);
		if (selectionCopy.hasChanged()) {
			selectionCopy.setAfter();
			history.add(selectionCopy);

			Vec3 avg = selection.getAveragePosition(spline, zero);
			path.setSelectedPoints(selection);
			axes.setPosition(avg);
			createInterface();
			update();
		}
	} else if (enabled && event->button() == Qt::LeftButton) {
		if (!selection.getPoints().empty()) {
			setClickOffset(p.x(), p.y(), axes.getPosition());
			moveSpline = MoveSpline(&selection, &origSpline, &axes);
			initiateMovePoint();
		}
	}
}

void CurveEditor::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::MidButton)
		camera.setAction(Camera::None);
	else if (move) {
		if (!extruding)
			history.add(moveSpline);
		else
			extruding = false;
		move = false;
	}

	axes.clearSelection();
	// emit editingFinished();
}

void CurveEditor::mouseMoveEvent(QMouseEvent *event)
{
	QPoint p = event->pos();
	float x = p.x() + clickOffset[0];
	float y = p.y() + clickOffset[1];

	camera.executeAction(p.x(), p.y());

	if (move) {
		ctrl = event->modifiers() & Qt::ControlModifier;
		moveSpline.set(camera.getRay(x, y), camera.getDirection());
		moveSpline.setParallelTangents(false);
		moveSpline.execute();
		spline = origSpline;
		applyRestrictions();
		createInterface();
		emit curveChanged(spline, name);
	}

	update();
}

void CurveEditor::initiateMovePoint()
{
	move = true;
	origSpline = spline;
	/* Restrictions need to be applied from left to right or right to left
	 * depending on the direction the points are dragged. */
	origPoint = spline.getControls()[*selection.getPoints().begin()];
}

void CurveEditor::applyRestrictions()
{
	auto controls = spline.getControls();
	moveLeft = controls[*selection.getPoints().begin()].x < origPoint.x;

	if (spline.getDegree() == 1) {
		restrictLinearControls();
	} else if (spline.getDegree() == 3) {
		restrictOuterCubicControls(controls);
		restrictCubicControls(controls);
		spline.setControls(controls);
		if (!ctrl)
			parallelizeTangents();
		restrictOppositeCubicControls();
	}
}

void CurveEditor::restrictLinearControls()
{
	auto points = selection.getPoints();
	auto controls = spline.getControls();
	int l = controls.size() - 1;

	controls[0].x = 0.0f;
	controls[l].x = 1.0f;

	if (moveLeft)
		for (auto it = points.begin(); it != points.end(); ++it)
			restrictLinearControl(controls, *it);
	else
		for (auto it = points.rbegin(); it != points.rend(); ++it)
			restrictLinearControl(controls, *it);

	spline.setControls(controls);
}

void CurveEditor::restrictLinearControl(std::vector<pg::Vec3>& controls, int i)
{
	int l = controls.size() - 1;
	if (controls[i].z > 1.0f)
			controls[i].z = 1.0f;
	if (controls[i].z < 0.0f)
		controls[i].z = 0.0f;

	if (i != 0 && i != l) {
		if (moveLeft) {
			if (i < l && controls[i].x > controls[i+1].x)
				controls[i].x = controls[i+1].x;
			if (i > 0 && controls[i].x < controls[i-1].x)
				controls[i].x = controls[i-1].x;
		} else {
			if (i > 0 && controls[i].x < controls[i-1].x)
				controls[i].x = controls[i-1].x;
			if (i < l && controls[i].x > controls[i+1].x)
				controls[i].x = controls[i+1].x;
		}
	}
}

/**
 * Restrict outer points and keep the tangents the same (the unrestricted
 * points are not bound to x = 0 or x = 1).
 */
void CurveEditor::restrictOuterCubicControls(std::vector<Vec3> &controls)
{
	auto last = controls.size() - 1;
	if (controls[last].x != 1.0f) {
		controls[last-1].x -= (controls[last].x - 1.0f);
		controls[last].x = 1.0f;
	}
	if (controls[last].z > 1.0f) {
		controls[last-1].z -= (controls[last].z - 1.0f);
		controls[last].z = 1.0f;
	} else if (controls[last].z < 0.0f) {
		controls[last-1].z -= controls[last].z;
		controls[last].z = 0.0f;
	}

	if (controls[0].x != 0.0f) {
		controls[1].x -= controls[0].x;
		controls[0].x = 0.0f;
	}
	if (controls[0].z > 1.0f) {
		controls[1].z -= (controls[0].z - 1.0f);
		controls[0].z = 1.0f;
	} else if (controls[0].z < 0.0f) {
		controls[1].z -= controls[0].z;
		controls[0].z = 0.0f;
	}
}

void CurveEditor::restrictCubicControls(std::vector<Vec3> &controls)
{
	auto points = selection.getPoints();
	if (moveLeft)
		for (auto it = points.begin(); it != points.end(); it++) {
			int remainder = *it % 3;
			bool a = it == points.begin();
			bool b = it == --points.end();
			if (remainder == 1 && (!a && *std::prev(it) == *it - 1))
				continue;
			if (remainder == 2 && (!b && *std::next(it) == *it + 1))
				continue;
			restrictCubicControl(controls, *it);
		}
	else
		for (auto it = points.rbegin(); it != points.rend(); it++) {
			int remainder = *it % 3;
			bool a = it == points.rbegin();
			bool b = it == --points.rend();
			if (remainder == 1 && (!b && *std::prev(it) == *it - 1))
				continue;
			if (remainder == 2 && (!a && *std::next(it) == *it + 1))
				continue;
			restrictCubicControl(controls, *it);
		}
}

void CurveEditor::restrictCubicControl(std::vector<Vec3> &controls, int i)
{
	int remainder = i % 3;

	if (controls[i].z > 1.0f)
		controls[i].z = 1.0f;
	else if (controls[i].z < 0.0f)
		controls[i].z = 0.0f;

	if (i != 0 && i != (int)controls.size() - 1) {
		if (remainder == 1) {
			if (controls[i].x < controls[i-1].x)
				controls[i].x = controls[i-1].x;
			if (controls[i].x > controls[i+2].x)
				controls[i].x = controls[i+2].x;
		} else if (remainder == 2) {
			if (controls[i].x < controls[i-2].x)
				controls[i].x = controls[i-2].x;
			if (controls[i].x > controls[i+1].x)
				controls[i].x = controls[i+1].x;
		} else {
			int l = controls.size() - 1;
			if (moveLeft) {
				if (i < l && controls[i].x > controls[i+2].x)
					controls[i].x = controls[i+2].x;
				if (i > 0 && controls[i].x < controls[i-2].x)
					controls[i].x = controls[i-2].x;
			} else {
				if (i > 0 && controls[i].x < controls[i-2].x)
					controls[i].x = controls[i-2].x;
				if (i < l && controls[i].x > controls[i+2].x)
					controls[i].x = controls[i+2].x;
			}

			if (i != 0 && i != l) {
				Vec3 orig = origSpline.getControls()[i];
				Vec3 diff = controls[i] - orig;
				controls[i+1] += diff;
				controls[i-1] += diff;
			}
		}
	}
}

bool CurveEditor::isCenterSelected(std::set<int>::iterator &it)
{
	auto points = selection.getPoints();
	int point = *it;
	int remainder = point % 3;
	auto n = std::next(it);
	auto p = std::prev(it);
	auto e = --points.end();
	auto b = points.begin();
	if (remainder == 1 && it != b && *p == point - 1)
		return true;
	else if (remainder == 2 && n != e && *n == point + 1)
		return true;
	else
		return false;
}

void CurveEditor::parallelizeTangents()
{
	auto points = selection.getPoints();
	for (auto it = points.begin(); it != points.end(); ++it) {
		if (!isCenterSelected(it)) {
			int point = *it;
			int remainder = point % 3;
			if (remainder == 1) {
				if (points.find(point - 2) == points.end())
					spline.parallelize(point);
			} else if (remainder == 2) {
				if (points.find(point + 2) == points.end())
					spline.parallelize(point);
			}
		}
	}
}

void CurveEditor::restrictOppositeCubicControls()
{
	std::vector<Vec3> controls = spline.getControls();
	auto points = selection.getPoints();
	for (auto it = points.begin(); it != points.end(); ++it) {
		int point = *it;
		int remainder = point % 3;
		if (remainder == 0) {
			if (point > 0)
				truncateCubicControl(controls, point - 1);
			if (point < (int)controls.size() - 1)
				truncateCubicControl(controls, point + 1);
		} else {
			int l = controls.size() - 2;
			if (!isCenterSelected(it) && point != 1 && point != l) {
				point += remainder == 1 ? -2 : 2;
				truncateCubicControl(controls, point);
			}
		}
	}
	spline.setControls(controls);
}

void CurveEditor::truncateCubicControl(std::vector<Vec3> &controls, int i)
{
	int j;
	int k;
	if (i % 3 == 1) {
		j = i - 1;
		k = i + 2;
	} else {
		j = i + 1;
		k = i - 2;
	}

	Vec3 diff = controls[i] - controls[j];
	if (!(diff.x == 0.0f && diff.z == 0.0f)) {
		float length = pg::magnitude(diff);

		pg::Ray2 tangent;
		tangent.origin = toVec2(controls[j]);
		tangent.direction = pg::toVec2(pg::normalize(diff));

		float t;
		float r;
		pg::Ray2 boundary;
		boundary.origin = (pg::Vec2){0.0f, 1.0f};
		boundary.direction = (pg::Vec2){1.0f, 0.0f};
		t = intersectsLine(tangent, boundary);
		/* Discard result if the direction is inward. */
		if (tangent.direction.y < 0)
			t = -1.0f;

		boundary.origin = (pg::Vec2){0.0f, 0.0f};
		boundary.direction = (pg::Vec2){1.0f, 0.0f};
		r = intersectsLine(tangent, boundary);
		/* Discard result if the direction is inward. */
		if (tangent.direction.y > 0)
			r = -1.0f;
		if (t < 0 || (r < t && r >= 0))
			t = r;

		boundary.origin = (pg::Vec2){controls[k].x, 0.0f};
		boundary.direction = (pg::Vec2){0.0f, 1.0f};
		r = intersectsLine(tangent, boundary);
		if (t < 0 || (r < t && r >= 0))
			t = r;

		if (length > t) {
			pg::Vec2 p = t * normalize(tangent.direction);
			p += tangent.origin;
			controls[i].x = p.x;
			controls[i].z = p.y;
		}
	}
}

void CurveEditor::paintGL()
{
	glClearColor(0.32f, 0.32f, 0.32f, 1.0);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Mat4 vp = camera.getVP();
	buffer.use();
	glUseProgram(shared->getShader(Shader::Flat));
	glUniformMatrix4fv(0, 1, GL_FALSE, &vp[0][0]);

	{
		auto size = sizeof(unsigned);
		GLvoid *start = (GLvoid *)(planeSegment.istart * size);
		GLsizei count = planeSegment.icount;
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, start);
	}

	glDrawArrays(GL_LINES, gridSegment.pstart, gridSegment.pcount);

	if (enabled && spline.getControls().size() > 0) {
		glUseProgram(shared->getShader(Shader::Line));
		glUniformMatrix4fv(0, 1, GL_FALSE, &vp[0][0]);
		glUniform2f(1, QWidget::width(), QWidget::height());

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

	glFlush();
}

void CurveEditor::setCurve(pg::Spline spline, QString name)
{
	parentWidget()->setWindowTitle(name + " Curve Editor");

	degree->blockSignals(true);
	switch(spline.getDegree()) {
	case 1:
		degree->setCurrentIndex(0);
		break;
	case 3:
		degree->setCurrentIndex(1);
		break;
	}
	degree->setEnabled(true);
	degree->blockSignals(false);

	this->spline = spline;
	this->name = name;
	selection.clear();

	history.clear();

	createInterface();
	update();
}

void CurveEditor::setDegree(int degree)
{
	degree = degree == 1 ? 3 : 1;
	if (degree != spline.getDegree()) {
		spline.adjust(degree);
		selection.clear();
		createInterface();
		history.clear();
		update();
		emit curveChanged(spline, name);
		emit editingFinished();
	}
}

void CurveEditor::setEnabled(bool enabled)
{
	if (enabled && name.size() > 0) {
		degree->setEnabled(true);
		parentWidget()->setWindowTitle(name + " Curve Editor");
	} else {
		degree->setEnabled(false);
		parentWidget()->setWindowTitle("Curve Editor");
	}

	this->enabled = enabled;
	update();
}

void CurveEditor::setClickOffset(int x, int y, pg::Vec3 point)
{
	pg::Vec3 s = camera.toScreenSpace(point);
	clickOffset[0] = s.x - x;
	clickOffset[1] = s.y - y;
}

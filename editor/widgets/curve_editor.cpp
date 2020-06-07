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

#include "curve_editor.h"
#include "editor/selector.h"
#include "editor/commands/extrude_spline.h"
#include "editor/commands/remove_spline.h"
#include "editor/commands/save_point_selection.h"
#include "editor/geometry/geometry.h"
#include "plant_generator/math/curve.h"
#include "plant_generator/math/intersection.h"
#include <QtGui/QMouseEvent>
#include <QTabBar>
#include <cmath>

using pg::Vec2;
using pg::Vec3;
using pg::Mat4;

CurveEditor::CurveEditor(
	SharedResources *shared, KeyMap *keymap, QWidget *parent) :
	QOpenGLWidget(parent)
{
	this->shared = shared;
	this->keymap = keymap;
	this->enabled = false;

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setSizeConstraint(QLayout::SetMinimumSize);
	layout->setSpacing(0);
	layout->setMargin(0);
	this->degree = new QComboBox(this);
	this->degree->addItem(QString("Linear"));
	this->degree->addItem(QString("Cubic"));
	this->degree->setFixedHeight(22);
	this->degree->setEnabled(false);
	layout->addWidget(this->degree);
	layout->addStretch(1);
	setFocusPolicy(Qt::StrongFocus);
	setMouseTracking(true);

	this->camera.setTarget(Vec3(0.5f, 0.0f, 0.5f));
	this->camera.setOrientation(180.0f, -180.0f);
	this->camera.setDistance(1.0f);
	this->camera.setPanSpeed(0.004f);
	this->camera.setZoom(0.01f, 0.3f, 2.0f);

	connect(this->degree, SIGNAL(currentIndexChanged(int)),
		this, SLOT(setDegree(int)));
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

	this->buffer.initialize(GL_DYNAMIC_DRAW);
	this->buffer.allocatePointMemory(1000);
	this->buffer.allocateIndexMemory(1000);

	this->path.setColor(
		Vec3(0.5f, 0.5f, 0.5f),
		Vec3(0.6f, 0.6f, 0.6f),
		Vec3(0.1f, 1.0f, 0.4f));
	glPrimitiveRestartIndex(Geometry::primitiveReset);

	change();
}

void CurveEditor::focusOutEvent(QFocusEvent *)
{
	emit editingFinished();
}

void CurveEditor::createInterface()
{
	Geometry geometry;

	{
		this->path.set(this->spline, 20, Vec3(0.0f, 0.0f, 0.0f));
		this->path.setSelectedPoints(selection);
		geometry.append(*this->path.getGeometry());
	}

	{
		Geometry plane;
		Vec3 a(1.0f, 0.0f, 0.0f);
		Vec3 b(0.0f, 0.0f, 1.0f);
		Vec3 center(0.0f, 0.2f, 0.0f);
		Vec3 color(0.34f, 0.34f, 0.34f);
		plane.addPlane(a, b, center, color);
		this->planeSegment = geometry.append(plane);
	}

	{
		Geometry grid;
		Vec3 colors[2];
		colors[0] = Vec3(0.3f, 0.3f, 0.3f);
		colors[1] = Vec3(0.3f, 0.3f, 0.3f);
		Mat4 t(
			1.0f/6.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f/6.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f/6.0f, 0.0f,
			0.5f, 0.0f, 0.5f, 1.0f);
		grid.addGrid(100, colors, colors[0]);
		Geometry::Segment segment = grid.getSegment();
		grid.transform(segment.pstart, segment.pcount, t);
		this->gridSegment = geometry.append(grid);
	}

	this->buffer.update(geometry);
}

void CurveEditor::resizeGL(int width, int height)
{
	height -= this->toolBarHeight;
	float ratio = static_cast<float>(width) / static_cast<float>(height);
	this->camera.setWindowSize(width, height);
	this->camera.setOrthographic(
		Vec3(-ratio, -1.0f, 0.0f), Vec3(ratio, 1.0f, 100.0f));
}

void CurveEditor::keyPressEvent(QKeyEvent *event)
{
	if (this->command)
		exitCommand(this->command->onKeyPress(event));

	unsigned key = event->key();
	bool ctrl = event->modifiers() & Qt::ControlModifier;
	bool shift = event->modifiers() & Qt::ShiftModifier;
	bool alt = event->modifiers() & Qt::AltModifier;
	QString command = this->keymap->getBinding(key, ctrl, shift, alt);

	if (command == tr("Select Previous Points")) {
		SavePointSelection *selectionCopy =
			new SavePointSelection(&this->selection);
		this->selection.selectNext(this->spline.getSize());
		if (selectionCopy->hasChanged()) {
			selectionCopy->setAfter();
			this->history.add(selectionCopy);
			change();
		}
	} else if (command == tr("Select Previous Points")) {
		SavePointSelection *selectionCopy =
			new SavePointSelection(&this->selection);
		this->selection.selectPrevious();
		if (selectionCopy->hasChanged()) {
			selectionCopy->setAfter();
			this->history.add(selectionCopy);
			change();
		}
	} else if (command == tr("Select All Points")) {
		SavePointSelection *selectionCopy =
			new SavePointSelection(&this->selection);
		this->selection.selectAll(this->spline.getSize());
		if (selectionCopy->hasChanged()) {
			selectionCopy->setAfter();
			this->history.add(selectionCopy);
			change();
		}
	} else if (command == tr("Extrude")) {
		extrude();
	} else if (command == tr("Remove")) {
		/* The first and last points cannot be removed. */
		auto points = this->selection.getPoints();
		int lastPoint = this->spline.getSize() - 1;
		points.erase(lastPoint);
		points.erase(0);
		if (this->spline.getDegree() == 3) {
			points.erase(lastPoint - 1);
			points.erase(1);
		}
		this->selection.setPoints(points);

		RemoveSpline *removeSpline =
			new RemoveSpline(&this->selection, &this->origSpline);
		removeSpline->execute();
		this->spline = this->origSpline;
		change();
		this->history.add(removeSpline);
		emit curveChanged(this->spline, this->name);
	} else if (key == Qt::Key_Z && ctrl) {
		if (event->modifiers() && shift)
			this->history.redo();
		else
			this->history.undo();
		this->spline = this->origSpline;
		applyRestrictions();
		change();
		emit curveChanged(this->spline, this->name);
	}
}

void CurveEditor::extrude()
{
	auto points = this->selection.getPoints();
	/* The last point cannot be extruded. */
	points.erase(this->spline.getSize() - 1);

	/* Extrusion should fail if tangents overlap. */
	if (this->spline.getDegree() == 3) {
		auto controls = this->spline.getControls();
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

	this->selection.setPoints(points);

	Vec3 avg = pg::getZeroVec3();
	avg = this->selection.getAveragePosition(this->spline, avg);
	this->axes.setPosition(avg);

	this->origSpline = this->spline;
	ExtrudeSpline *extrude = new ExtrudeSpline(
		&this->selection, &this->origSpline, &this->axes,
		&this->camera);
	pg::Vec3 s = this->camera.toScreenSpace(avg);
	QPoint pos = mapFromGlobal(QCursor::pos());
	extrude->setClickOffset(s.x - pos.x(), s.y - pos.y());
	extrude->execute();
	this->command = extrude;
	this->spline = this->origSpline;

	initiateMovePoint();
	applyRestrictions();
	change();

	emit curveChanged(this->spline, this->name);
}

void CurveEditor::mousePressEvent(QMouseEvent *event)
{
	QPoint pos = event->pos();

	if (this->command) {
		exitCommand(this->command->onMousePress(event));
	} else if (event->button() == Qt::MidButton) {
		int x = pos.x();
		int y = pos.y() - this->toolBarHeight;
		this->camera.setStartCoordinates(x, y);
		if (event->modifiers() & Qt::ControlModifier)
			this->camera.setAction(Camera::Zoom);
		else if (event->modifiers() & Qt::ShiftModifier)
			this->camera.setAction(Camera::Pan);
	} else if (this->enabled && event->button() == Qt::RightButton) {
		SavePointSelection *selectionCopy =
			new SavePointSelection(&this->selection);
		QMouseEvent modifiedEvent(
			QEvent::MouseMove,
			QPoint(pos.x(), pos.y() - this->toolBarHeight),
			event->button(), event->buttons(), event->modifiers());
		Selector selector(&this->camera);
		selector.selectPoint(
			&modifiedEvent, this->spline, pg::getZeroVec3(),
			&this->selection);
		if (selectionCopy->hasChanged()) {
			selectionCopy->setAfter();
			this->history.add(selectionCopy);
			Vec3 avg = this->selection.getAveragePosition(
				this->spline, pg::getZeroVec3());
			this->path.setSelectedPoints(this->selection);
			this->axes.setPosition(avg);
			change();
		}
	} else if (this->enabled && event->button() == Qt::LeftButton) {
		if (!this->selection.getPoints().empty()) {
			Vec3 s = this->axes.getPosition();
			s = this->camera.toScreenSpace(s);
			int x = s.x - pos.x();
			int y = s.y - pos.y();
			initiateMovePoint();
			MoveSpline *moveSpline = new MoveSpline(
				&this->selection, &this->origSpline,
				&this->axes, &this->camera);
			moveSpline->preservePositions();
			moveSpline->setClickOffset(x, y);
			this->command = moveSpline;
		}
	}
}

void CurveEditor::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::MidButton)
		this->camera.setAction(Camera::None);
	else if (this->command)
		exitCommand(this->command->onMouseRelease(event));

	this->axes.clearSelection();
}

void CurveEditor::mouseMoveEvent(QMouseEvent *event)
{
	QPoint p = event->pos();
	float x = p.x();
	float y = p.y();

	if (this->command) {
		this->ctrl = event->modifiers() & Qt::ControlModifier;
		if (this->command->onMouseMove(event)) {
			this->spline = this->origSpline;
			applyRestrictions();
			change();
			emit curveChanged(this->spline, this->name);
		}
	} else if (this->camera.executeAction(x, y - this->toolBarHeight))
		change();
}

void CurveEditor::wheelEvent(QWheelEvent *event)
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

void CurveEditor::initiateMovePoint()
{
	this->origSpline = this->spline;
	/* Restrictions need to be applied from left to right or right to left
	depending on the direction the points are dragged. */
	int point = *this->selection.getPoints().begin();
	this->origPoint = this->spline.getControls()[point];
}

void CurveEditor::applyRestrictions()
{
	if (this->selection.getPoints().empty())
		return;

	auto controls = this->spline.getControls();
	int point = *this->selection.getPoints().begin();
	this->moveLeft = controls[point].x < this->origPoint.x;

	if (this->spline.getDegree() == 1) {
		restrictLinearControls();
	} else if (this->spline.getDegree() == 3) {
		restrictOuterCubicControls(controls);
		restrictCubicControls(controls);
		this->spline.setControls(controls);
		if (!this->ctrl)
			parallelizeTangents();
		restrictOppositeCubicControls();
	}
}

void CurveEditor::restrictLinearControls()
{
	auto points = this->selection.getPoints();
	auto controls = this->spline.getControls();

	controls[0].x = 0.0f;
	controls[controls.size() - 1].x = 1.0f;

	if (this->moveLeft)
		for (auto it = points.begin(); it != points.end(); ++it)
			restrictLinearControl(controls, *it);
	else
		for (auto it = points.rbegin(); it != points.rend(); ++it)
			restrictLinearControl(controls, *it);

	this->spline.setControls(controls);
}

void CurveEditor::restrictLinearControl(std::vector<pg::Vec3>& controls, int i)
{
	int l = controls.size() - 1;
	if (controls[i].z > 1.0f)
			controls[i].z = 1.0f;
	if (controls[i].z < 0.0f)
		controls[i].z = 0.0f;

	if (i != 0 && i != l) {
		if (this->moveLeft) {
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

/** Restrict outer points and keep the tangents the same (the unrestricted
points are not bound to x = 0 or x = 1). */
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
	auto points = this->selection.getPoints();
	if (this->moveLeft)
		for (auto it = points.begin(); it != points.end(); it++) {
			int point = *it;
			int prevPoint = *std::prev(it);
			int nextPoint = *std::next(it);
			bool a = it == points.begin();
			bool b = it == --points.end();
			if (point % 3 == 1 && (!a && prevPoint == point - 1))
				continue;
			if (point % 3 == 2 && (!b && nextPoint == point + 1))
				continue;
			restrictCubicControl(controls, *it);
		}
	else
		for (auto it = points.rbegin(); it != points.rend(); it++) {
			int point = *it;
			int prevPoint = *std::prev(it);
			int nextPoint = *std::next(it);
			bool a = it == points.rbegin();
			bool b = it == --points.rend();
			if (point % 3 == 1 && (!b && prevPoint == point - 1))
				continue;
			if (point % 3 == 2 && (!a && nextPoint == point + 1))
				continue;
			restrictCubicControl(controls, *it);
		}
}

void CurveEditor::restrictCubicControl(std::vector<Vec3> &controls, int i)
{
	if (controls[i].z > 1.0f)
		controls[i].z = 1.0f;
	else if (controls[i].z < 0.0f)
		controls[i].z = 0.0f;

	if (i != 0 && i != (int)controls.size() - 1) {
		if (i % 3 == 1) {
			if (controls[i].x < controls[i-1].x)
				controls[i].x = controls[i-1].x;
			if (controls[i].x > controls[i+2].x)
				controls[i].x = controls[i+2].x;
		} else if (i % 3 == 2) {
			if (controls[i].x < controls[i-2].x)
				controls[i].x = controls[i-2].x;
			if (controls[i].x > controls[i+1].x)
				controls[i].x = controls[i+1].x;
		} else {
			int l = controls.size() - 1;
			if (this->moveLeft) {
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
				Vec3 orig = this->origSpline.getControls()[i];
				Vec3 diff = controls[i] - orig;
				controls[i+1] += diff;
				controls[i-1] += diff;
			}
		}
	}
}

bool CurveEditor::isCenterSelected(std::set<int>::iterator &it)
{
	auto points = this->selection.getPoints();
	int point = *it;
	auto n = std::next(it);
	auto p = std::prev(it);
	auto e = --points.end();
	auto b = points.begin();
	if (point % 3 == 1 && it != b && *p == point - 1)
		return true;
	else if (point % 3 == 2 && n != e && *n == point + 1)
		return true;
	else
		return false;
}

void CurveEditor::parallelizeTangents()
{
	auto points = this->selection.getPoints();
	for (auto it = points.begin(); it != points.end(); ++it) {
		if (!isCenterSelected(it)) {
			int point = *it;
			if (point % 3 == 1) {
				if (points.find(point - 2) == points.end())
					this->spline.parallelize(point);
			} else if (point % 3 == 2) {
				if (points.find(point + 2) == points.end())
					this->spline.parallelize(point);
			}
		}
	}
}

void CurveEditor::restrictOppositeCubicControls()
{
	std::vector<Vec3> controls = this->spline.getControls();
	auto points = this->selection.getPoints();
	for (auto it = points.begin(); it != points.end(); ++it) {
		int point = *it;
		if (point % 3 == 0) {
			if (point > 0)
				truncateCubicControl(controls, point - 1);
			if (point < (int)controls.size() - 1)
				truncateCubicControl(controls, point + 1);
		} else {
			int l = controls.size() - 2;
			if (!isCenterSelected(it) && point != 1 && point != l) {
				point += (point % 3 == 1) ? -2 : 2;
				truncateCubicControl(controls, point);
			}
		}
	}
	this->spline.setControls(controls);
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
		tangent.origin = pg::toVec2(controls[j]);
		tangent.direction = pg::toVec2(pg::normalize(diff));

		float t;
		float r;
		pg::Ray2 boundary;
		boundary.origin = Vec2(0.0f, 1.0f);
		boundary.direction = Vec2(1.0f, 0.0f);
		t = intersectsLine(tangent, boundary);
		/* Discard result if the direction is inward. */
		if (tangent.direction.y < 0)
			t = -1.0f;

		boundary.origin =  Vec2(0.0f, 0.0f);
		boundary.direction =  Vec2(1.0f, 0.0f);
		r = intersectsLine(tangent, boundary);
		/* Discard result if the direction is inward. */
		if (tangent.direction.y > 0)
			r = -1.0f;
		if (t < 0 || (r < t && r >= 0))
			t = r;

		boundary.origin =  Vec2(controls[k].x, 0.0f);
		boundary.direction =  Vec2(0.0f, 1.0f);
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
	glViewport(0, 0, width(), height() - this->toolBarHeight);
	glClearColor(0.32f, 0.32f, 0.32f, 1.0);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	this->camera.updateVP();
	Mat4 vp = this->camera.getVP();

	this->buffer.use();
	glUseProgram(this->shared->getShader(SharedResources::Flat));
	glUniformMatrix4fv(0, 1, GL_FALSE, &vp[0][0]);

	{
		auto size = sizeof(unsigned);
		GLvoid *start = (GLvoid *)(this->planeSegment.istart * size);
		GLsizei count = this->planeSegment.icount;
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, start);
	}

	glDrawArrays(GL_LINES, this->gridSegment.pstart,
		this->gridSegment.pcount);

	if (this->enabled && this->spline.getControls().size() > 0) {
		glUseProgram(this->shared->getShader(SharedResources::Line));
		glUniformMatrix4fv(0, 1, GL_FALSE, &vp[0][0]);
		glUniform2f(1, QWidget::width(), QWidget::height());

		Geometry::Segment segment = this->path.getLineSegment();
		GLvoid *start = (GLvoid *)(segment.istart * sizeof(unsigned));
		glDrawElements(
			GL_LINE_STRIP, segment.icount, GL_UNSIGNED_INT, start);

		segment = this->path.getPointSegment();
		GLuint texture =
			this->shared->getTexture(SharedResources::DotTexture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUseProgram(this->shared->getShader(SharedResources::Point));
		glUniformMatrix4fv(0, 1, GL_FALSE, &vp[0][0]);
		glDrawArrays(GL_POINTS, segment.pstart, segment.pcount);
	}

	glFlush();
}

void CurveEditor::change()
{
	createInterface();
	update();
}

void CurveEditor::exitCommand(bool changed)
{
	if (changed)
		change();

	if (this->command->isDone()) {
		this->history.add(this->command);
		this->command = nullptr;
	}
}

void CurveEditor::setCurve(pg::Spline spline, QString name)
{
	parentWidget()->setWindowTitle(name + " Curve");

	this->degree->blockSignals(true);
	if (spline.getDegree() == 1)
		this->degree->setCurrentIndex(0);
	else if (spline.getDegree() == 3)
		this->degree->setCurrentIndex(1);
	this->degree->setEnabled(true);
	this->degree->blockSignals(false);

	this->spline = spline;
	this->origSpline = spline;
	this->name = name;
	this->selection.clear();
	this->history.clear();

	change();
}

void CurveEditor::setDegree(int degree)
{
	degree = degree == 1 ? 3 : 1;
	if (degree != this->spline.getDegree()) {
		this->spline.adjust(degree);
		this->selection.clear();
		this->history.clear();
		change();
		emit curveChanged(this->spline, this->name);
		emit editingFinished();
	}
}

void CurveEditor::setEnabled(bool enabled)
{
	if (enabled && this->name.size() > 0) {
		this->degree->setEnabled(true);
		parentWidget()->setWindowTitle(this->name + " Curve");
	} else {
		this->degree->setEnabled(false);
		parentWidget()->setWindowTitle("Curves");
	}

	this->enabled = enabled;
	update();
}

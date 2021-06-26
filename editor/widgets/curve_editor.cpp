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
#include "form.h"
#include "editor/selector.h"
#include "editor/commands/extrude_spline.h"
#include "editor/commands/remove_spline.h"
#include "editor/commands/save_point_selection.h"
#include "editor/geometry/geometry.h"
#include "plant_generator/math/curve.h"
#include "plant_generator/math/intersection.h"
#include <cmath>

using pg::Vec2;
using pg::Vec3;
using pg::Mat4;

CurveEditor::CurveEditor(KeyMap *keymap, QWidget *parent) :
	QWidget(parent),
	keymap(keymap),
	layout(new QVBoxLayout(this)),
	degree(new ComboBox(this)),
	command(nullptr)
{
	this->layout->setSizeConstraint(QLayout::SetMinimumSize);
	this->layout->setSpacing(0);
	this->layout->setMargin(0);
}

void CurveEditor::createInterface(SharedResources *shared)
{
	this->degree->addItem(QString("Linear"));
	this->degree->addItem(QString("Cubic"));
	this->degree->setFixedHeight(UI_FIELD_HEIGHT);
	this->layout->addWidget(this->degree);
	connect(this->degree,
		QOverload<int>::of(&ComboBox::currentIndexChanged),
		this, &CurveEditor::setDegree);
	this->viewer = new CurveViewer(shared, this);
	this->viewer->installEventFilter(this);
	this->layout->addWidget(this->viewer);
}

QSize CurveEditor::sizeHint() const
{
	return QSize(UI_WIDGET_WIDTH, UI_WIDGET_WIDTH);
}

const CurveViewer *CurveEditor::getViewer() const
{
	return this->viewer;
}

void CurveEditor::focusOutEvent(QFocusEvent *)
{
	emit editingFinished();
}

bool CurveEditor::eventFilter(QObject *object, QEvent *event)
{
	bool accepted = QWidget::eventFilter(object, event);
	if (object != this->viewer)
		return accepted;
	if (event->type() == QEvent::MouseButtonPress)
		mousePressed(static_cast<QMouseEvent *>(event));
	else if (event->type() == QEvent::MouseMove)
		mouseMoved(static_cast<QMouseEvent *>(event));
	else if (event->type() == QEvent::MouseButtonRelease)
		mouseReleased(static_cast<QMouseEvent *>(event));
	return accepted;
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

	if (command == "Select Next Points") {
		SavePointSelection *selectionCopy = new SavePointSelection(
			&this->selection);
		this->selection.selectNext(this->spline.getSize());
		if (selectionCopy->hasChanged()) {
			selectionCopy->setAfter();
			this->history.add(selectionCopy);
			change(false);
		} else
			delete selectionCopy;
	} else if (command == "Select Previous Points") {
		SavePointSelection *selectionCopy = new SavePointSelection(
			&this->selection);
		this->selection.selectPrevious();
		if (selectionCopy->hasChanged()) {
			selectionCopy->setAfter();
			this->history.add(selectionCopy);
			change(false);
		} else
			delete selectionCopy;
	} else if (command == "Select Points") {
		SavePointSelection *selectionCopy = new SavePointSelection(
			&this->selection);
		this->selection.selectAll(this->spline.getSize());
		if (selectionCopy->hasChanged()) {
			selectionCopy->setAfter();
			this->history.add(selectionCopy);
			change(false);
		} else
			delete selectionCopy;
	} else if (command == "Extrude") {
		extrude();
	} else if (command == "Remove") {
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
		RemoveSpline *removeSpline = new RemoveSpline(&this->selection,
			&this->originalSpline);
		removeSpline->execute();
		this->spline = this->originalSpline;
		change(true);
		this->history.add(removeSpline);
	} else if (key == Qt::Key_Z && ctrl) {
		if (event->modifiers() && shift)
			this->history.redo();
		else
			this->history.undo();
		bool curveChanged = this->spline != this->originalSpline;
		this->spline = this->originalSpline;
		applyRestrictions();
		change(curveChanged);
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

	Vec3 avg(0.0f, 0.0f, 0.0f);
	avg = this->selection.getAveragePosition(this->spline, avg);
	this->axes.setPosition(avg);

	this->originalSpline = this->spline;
	const Camera *camera = this->viewer->getCamera();
	ExtrudeSpline *extrude = new ExtrudeSpline(&this->selection,
		&this->originalSpline, &this->axes, camera);
	pg::Vec3 s = camera->toScreenSpace(avg);
	QPoint pos = this->viewer->mapFromGlobal(QCursor::pos());
	extrude->setClickOffset(s.x - pos.x(), s.y - pos.y());
	extrude->execute();
	this->command = extrude;
	this->spline = this->originalSpline;

	initiateMovePoint();
	applyRestrictions();
	change(true);
}

void CurveEditor::mousePressed(QMouseEvent *event)
{
	QPoint pos = event->pos();
	const Camera *camera = this->viewer->getCamera();

	if (this->command)
		exitCommand(this->command->onMousePress(event));
	else if (event->button() == Qt::RightButton) {
		SavePointSelection *selectionCopy = new SavePointSelection(
			&this->selection);

		Selector selector(camera);
		selector.selectPoint(event, this->spline,
			Vec3(0.0, 0.0f, 0.0f), &this->selection);

		if (selectionCopy->hasChanged()) {
			selectionCopy->setAfter();
			this->history.add(selectionCopy);
			Vec3 avg = this->selection.getAveragePosition(
				this->spline, Vec3(0.0f, 0.0f, 0.0f));
			this->axes.setPosition(avg);
			change(false);
		}
	} else if (event->button() == Qt::LeftButton) {
		if (!this->selection.getPoints().empty()) {
			Vec3 s = this->axes.getPosition();
			s = camera->toScreenSpace(s);
			int x = s.x - pos.x();
			int y = s.y - pos.y();
			initiateMovePoint();
			MoveSpline *moveSpline = new MoveSpline(
				&this->selection, &this->originalSpline,
				&this->axes, camera);
			moveSpline->preservePositions();
			moveSpline->setClickOffset(x, y);
			this->command = moveSpline;
		}
	}
}

void CurveEditor::initiateMovePoint()
{
	this->originalSpline = this->spline;
	/* Restrictions need to be applied from left to right or right to left
	depending on the direction the points are dragged. */
	int point = *this->selection.getPoints().begin();
	this->originalPoint = this->spline.getControls()[point];
}

void CurveEditor::mouseReleased(QMouseEvent *event)
{
	if (this->command)
		exitCommand(this->command->onMouseRelease(event));
	this->axes.clearSelection();
}

void CurveEditor::mouseMoved(QMouseEvent *event)
{
	if (this->command) {
		this->ctrl = event->modifiers() & Qt::ControlModifier;
		if (this->command->onMouseMove(event)) {
			this->spline = this->originalSpline;
			applyRestrictions();
			change(true);
		}
	}
}

void CurveEditor::applyRestrictions()
{
	if (this->selection.getPoints().empty())
		return;

	auto controls = this->spline.getControls();
	int point = *this->selection.getPoints().begin();
	this->moveLeft = controls[point].x < this->originalPoint.x;

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
	controls[controls.size()-1].x = 1.0f;
	controls[0].x = 0.0f;

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
	if (controls[i].y > 1.0f)
		controls[i].y = 1.0f;
	if (controls[i].y < 0.0f)
		controls[i].y = 0.0f;

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

/** Restrict the first and last points and keep the tangents the same (the
unrestricted points are not bound to x = 0 or x = 1). */
void CurveEditor::restrictOuterCubicControls(std::vector<Vec3> &controls)
{
	auto last = controls.size() - 1;

	if (controls[last].x != 1.0f) {
		controls[last-1].x -= (controls[last].x - 1.0f);
		controls[last].x = 1.0f;
	}

	if (controls[last].y > 1.0f) {
		controls[last-1].y -= (controls[last].y - 1.0f);
		controls[last].y = 1.0f;
	} else if (controls[last].y < 0.0f) {
		controls[last-1].y -= controls[last].y;
		controls[last].y = 0.0f;
	}

	if (controls[0].x != 0.0f) {
		controls[1].x -= controls[0].x;
		controls[0].x = 0.0f;
	}

	if (controls[0].y > 1.0f) {
		controls[1].y -= (controls[0].y - 1.0f);
		controls[0].y = 1.0f;
	} else if (controls[0].y < 0.0f) {
		controls[1].y -= controls[0].y;
		controls[0].y = 0.0f;
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
	if (controls[i].y > 1.0f)
		controls[i].y = 1.0f;
	else if (controls[i].y < 0.0f)
		controls[i].y = 0.0f;

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
				Vec3 p = this->originalSpline.getControls()[i];
				Vec3 diff = controls[i] - p;
				controls[i+1] += diff;
				controls[i-1] += diff;
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
	if (diff.x != 0.0f || diff.y != 0.0f) {
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
		if (tangent.direction.y < 0)
			t = -1.0f;

		boundary.origin = Vec2(0.0f, 0.0f);
		boundary.direction = Vec2(1.0f, 0.0f);
		r = intersectsLine(tangent, boundary);
		if (tangent.direction.y > 0)
			r = -1.0f;
		if (t < 0 || (r < t && r >= 0))
			t = r;

		boundary.origin = Vec2(controls[k].x, 0.0f);
		boundary.direction = Vec2(0.0f, 1.0f);
		r = intersectsLine(tangent, boundary);
		if (t < 0 || (r < t && r >= 0))
			t = r;

		if (length > t) {
			Vec2 p = t * pg::normalize(tangent.direction);
			p += tangent.origin;
			controls[i].x = p.x;
			controls[i].y = p.y;
		}
	}
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

void CurveEditor::exitCommand(bool changed)
{
	if (changed)
		change(false);
	if (this->command->isDone()) {
		this->history.add(this->command);
		this->command = nullptr;
	}
}

void CurveEditor::setDegree(int degree)
{
	degree = degree == 1 ? 3 : 1;
	if (degree != this->spline.getDegree()) {
		this->spline.adjust(degree);
		this->originalSpline = this->spline;
		this->selection.clear();
		this->history.clear();
		change(true);
		emit editingFinished();
	}
}

void CurveEditor::setSpline(const pg::Spline &spline)
{
	this->spline = spline;
	this->originalSpline = spline;
	this->viewer->change(this->spline, this->selection);
}

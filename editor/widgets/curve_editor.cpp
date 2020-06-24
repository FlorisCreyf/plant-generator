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
#include "definitions.h"
#include "editor/selector.h"
#include "editor/commands/extrude_spline.h"
#include "editor/commands/remove_spline.h"
#include "editor/commands/save_point_selection.h"
#include "editor/geometry/geometry.h"
#include "plant_generator/patterns.h"
#include "plant_generator/math/curve.h"
#include "plant_generator/math/intersection.h"
#include <QtGui/QMouseEvent>
#include <QTabBar>
#include <cmath>

using pg::Vec2;
using pg::Vec3;
using pg::Mat4;

CurveEditor::CurveEditor(
	SharedResources *shared, KeyMap *keymap, Editor *editor,
	QWidget *parent) : ObjectEditor(parent), editor(editor), keymap(keymap)
{
	this->command = nullptr;

	this->degree = new QComboBox(this);
	this->degree->addItem(QString("Linear"));
	this->degree->addItem(QString("Cubic"));
	this->degree->setFixedHeight(UI_FIELD_HEIGHT);
	this->layout->addWidget(this->degree);

	this->viewer = new CurveViewer(shared, this);
	this->layout->addWidget(this->viewer);
	connect(this->viewer, SIGNAL(ready()), this, SLOT(select()));

	connect(this->viewer, SIGNAL(mousePressed(QMouseEvent *)),
		this, SLOT(mousePressed(QMouseEvent *)));
	connect(this->viewer, SIGNAL(mouseReleased(QMouseEvent *)),
		this, SLOT(mouseReleased(QMouseEvent *)));
	connect(this->viewer, SIGNAL(mouseMoved(QMouseEvent *)),
		this, SLOT(mouseMoved(QMouseEvent *)));
	connect(this->degree, SIGNAL(currentIndexChanged(int)),
		this, SLOT(setDegree(int)));
}

CurveViewer *CurveEditor::getViewer() const
{
	return this->viewer;
}

void CurveEditor::init(const std::vector<pg::Curve> &curves)
{
	clear();
	this->selectionBox->blockSignals(true);
	for (pg::Curve curve : curves) {
		QString name = QString::fromStdString(curve.getName());
		this->selectionBox->addItem(name);
		emit curveAdded(curve);
	}
	this->selectionBox->blockSignals(false);
	select();
}

void CurveEditor::add()
{
	pg::Curve curve;
	std::string name;
	QString qname;

	for (int i = 1; true; i++) {
		name = "Curve " + std::to_string(i);
		qname = QString::fromStdString(name);
		if (this->selectionBox->findText(qname) == -1)
			break;
	}

	curve.setName(name);
	curve.setSpline(pg::getDefaultCurve(0));
	add(curve);
}

void CurveEditor::add(pg::Curve curve)
{
	this->selection.clear();
	this->history.clear();
	pg::Plant *plant = this->editor->getPlant();
	plant->addCurve(curve);

	this->selectionBox->blockSignals(true);
	QString name = QString::fromStdString(curve.getName());
	this->selectionBox->addItem(name);
	this->selectionBox->setCurrentIndex(this->selectionBox->findText(name));
	this->selectionBox->blockSignals(false);

	select();
	emit curveAdded(curve);
}

void CurveEditor::select()
{
	if (this->selectionBox->count()) {
		pg::Plant *plant = editor->getPlant();
		unsigned index = this->selectionBox->currentIndex();
		pg::Curve curve = plant->getCurve(index);
		this->spline = curve.getSpline();

		this->degree->blockSignals(true);
		int degree = this->spline.getDegree();
		this->degree->setCurrentIndex(degree == 3 ? 1 : 0);
		this->degree->blockSignals(false);

		this->viewer->change(this->spline, this->selection);
	}
}

void CurveEditor::rename()
{
	unsigned index = this->selectionBox->currentIndex();
	QString name = this->selectionBox->itemText(index);
	pg::Plant *plant = this->editor->getPlant();
	pg::Curve curve = plant->getCurve(index);
	curve.setName(name.toStdString());
	plant->updateCurve(curve, index);
	emit curveModified(curve, index);
}

void CurveEditor::remove()
{
	if (this->selectionBox->count() > 1) {
		unsigned index = this->selectionBox->currentIndex();
		QString name = this->selectionBox->currentText();
		pg::Plant *plant = this->editor->getPlant();
		plant->removeCurve(index);
		this->selectionBox->removeItem(index);
		select();
		this->editor->change();
		emit curveRemoved(index);
	}
}

void CurveEditor::clear()
{
	pg::Plant *plant = this->editor->getPlant();
	int count = this->selectionBox->count();
	while (count > 0) {
		plant->removeCurve(count-1);
		emit curveRemoved(count-1);
		count--;
	}
	this->selectionBox->clear();
	this->selection.clear();
	this->history.clear();
}

void CurveEditor::focusOutEvent(QFocusEvent *)
{
	emit editingFinished();
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
	} else if (key == Qt::Key_Z && ctrl) {
		if (event->modifiers() && shift)
			this->history.redo();
		else
			this->history.undo();
		this->spline = this->origSpline;
		applyRestrictions();
		change();
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

	this->origSpline = this->spline;
	const Camera *camera = this->viewer->getCamera();
	ExtrudeSpline *extrude = new ExtrudeSpline(
		&this->selection, &this->origSpline, &this->axes, camera);
	pg::Vec3 s = camera->toScreenSpace(avg);
	QPoint pos = this->viewer->mapFromGlobal(QCursor::pos());
	extrude->setClickOffset(s.x - pos.x(), s.y - pos.y());
	extrude->execute();
	this->command = extrude;
	this->spline = this->origSpline;

	initiateMovePoint();
	applyRestrictions();
	change();
}

void CurveEditor::mousePressed(QMouseEvent *event)
{
	QPoint pos = event->pos();
	const Camera *camera = this->viewer->getCamera();

	if (this->command)
		exitCommand(this->command->onMousePress(event));
	else if (event->button() == Qt::RightButton) {
		SavePointSelection *selectionCopy =
			new SavePointSelection(&this->selection);

		Selector selector(camera);
		selector.selectPoint(
			event, this->spline, Vec3(0.0, 0.0f, 0.0f),
			&this->selection);

		if (selectionCopy->hasChanged()) {
			selectionCopy->setAfter();
			this->history.add(selectionCopy);
			Vec3 avg = this->selection.getAveragePosition(
				this->spline, Vec3(0.0f, 0.0f, 0.0f));
			this->axes.setPosition(avg);
			change();
		}
	} else if (event->button() == Qt::LeftButton) {
		if (!this->selection.getPoints().empty()) {
			Vec3 s = this->axes.getPosition();
			s = camera->toScreenSpace(s);
			int x = s.x - pos.x();
			int y = s.y - pos.y();
			initiateMovePoint();
			MoveSpline *moveSpline = new MoveSpline(
				&this->selection, &this->origSpline,
				&this->axes, camera);
			moveSpline->preservePositions();
			moveSpline->setClickOffset(x, y);
			this->command = moveSpline;
		}
	}
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
			this->spline = this->origSpline;
			applyRestrictions();
			change();
		}
	}
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

		boundary.origin = Vec2(0.0f, 0.0f);
		boundary.direction = Vec2(1.0f, 0.0f);
		r = intersectsLine(tangent, boundary);
		/* Discard result if the direction is inward. */
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
			pg::Vec2 p = t * pg::normalize(tangent.direction);
			p += tangent.origin;
			controls[i].x = p.x;
			controls[i].z = p.y;
		}
	}
}

void CurveEditor::change()
{
	this->viewer->change(this->spline, this->selection);
	pg::Plant *plant = this->editor->getPlant();
	unsigned index = this->selectionBox->currentIndex();
	pg::Curve curve = plant->getCurve(index);
	curve.setSpline(this->spline);
	plant->updateCurve(curve, index);
	this->editor->change();
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

void CurveEditor::setCurve(pg::Spline spline)
{
	this->degree->blockSignals(true);
	if (spline.getDegree() == 1)
		this->degree->setCurrentIndex(0);
	else if (spline.getDegree() == 3)
		this->degree->setCurrentIndex(1);
	this->degree->blockSignals(false);

	this->spline = spline;
	this->origSpline = spline;
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
		emit editingFinished();
	}
}

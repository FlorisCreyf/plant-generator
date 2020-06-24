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

#ifndef CURVE_EDITOR_H
#define CURVE_EDITOR_H

#include "curve_viewer.h"
#include "editor.h"
#include "object_editor.h"
#include "editor/history.h"
#include "editor/keymap.h"
#include "editor/commands/move_spline.h"
#include "editor/geometry/translation_axes.h"
#include "plant_generator/curve.h"
#include "plant_generator/math/math.h"
#include <vector>

class CurveEditor : public ObjectEditor {
	Q_OBJECT

public:
	CurveEditor(SharedResources *shared, KeyMap *keymap, Editor *editor,
		QWidget *parent = 0);
	void setCurve(pg::Spline spline);
	CurveViewer *getViewer() const;
	void clear();

public slots:
	void add();
	void add(pg::Curve curve);
	void init(const std::vector<pg::Curve> &curves);
	void select();
	void rename();
	void remove();
	void setDegree(int degree);
	void mousePressed(QMouseEvent *event);
	void mouseReleased(QMouseEvent *event);
	void mouseMoved(QMouseEvent *event);

signals:
	void curveAdded(pg::Curve curve);
	void curveModified(pg::Curve curve, unsigned index);
	void curveRemoved(unsigned index);
	void editingFinished();

private:
	CurveViewer *viewer;
	Editor *editor;
	KeyMap *keymap;
	PointSelection selection;
	History history;
	TranslationAxes axes;

	pg::Spline spline;
	pg::Spline origSpline; /* The original spline without restrictions. */
	Command *command;

	bool ctrl;
	bool moveLeft;
	pg::Vec3 origPoint;

	QComboBox *degree;

	void initiateMovePoint();
	void applyRestrictions();
	void restrictLinearControls();
	void restrictLinearControl(std::vector<pg::Vec3> &, int);
	void restrictOuterCubicControls(std::vector<pg::Vec3> &);
	void restrictCubicControls(std::vector<pg::Vec3> &);
	void restrictCubicControl(std::vector<pg::Vec3> &, int);
	bool isCenterSelected(std::set<int>::iterator &);
	void parallelizeTangents();
	void restrictOppositeCubicControls();
	void truncateCubicControl(std::vector<pg::Vec3> &, int);
	void extrude();

	void focusOutEvent(QFocusEvent *);
	void keyPressEvent(QKeyEvent *);
	void exitCommand(bool);
	void change();
};

#endif /* CURVE_EDITOR_H */

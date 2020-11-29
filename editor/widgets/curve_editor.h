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
#include "editor/history.h"
#include "editor/keymap.h"
#include "editor/commands/move_spline.h"
#include "editor/geometry/translation_axes.h"
#include "plant_generator/curve.h"
#include <vector>
#include <QtWidgets>

class CurveEditor : public QWidget {
	Q_OBJECT

	KeyMap *keymap;
	TranslationAxes axes;
	bool ctrl;
	bool moveLeft;
	pg::Vec3 originalPoint;

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

	void mousePressed(QMouseEvent *event);
	void mouseReleased(QMouseEvent *event);
	void mouseMoved(QMouseEvent *event);
	void focusOutEvent(QFocusEvent *);
	void keyPressEvent(QKeyEvent *);
	void exitCommand(bool);

protected:
	QVBoxLayout *layout;
	QComboBox *degree;
	PointSelection selection;
	pg::Spline spline;
	/* The original spline without restrictions. */
	pg::Spline originalSpline;
	Command *command;
	CurveViewer *viewer;
	History history;

	CurveEditor(KeyMap *keymap, QWidget *parent);
	bool eventFilter(QObject *object, QEvent *event);
	void createInterface(SharedResources *shared);
	void setSpline(const pg::Spline &spline);
	virtual void change(bool curveChanged) = 0;

public:
	const CurveViewer *getViewer() const;
	QSize sizeHint() const;

public slots:
	void setDegree(int degree);

signals:
	void editingFinished();
};

#endif

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

#ifndef CURVE_EDITOR_H
#define CURVE_EDITOR_H

#include "editor/camera.h"
#include "editor/history.h"
#include "editor/keymap.h"
#include "editor/point_selection.h"
#include "editor/commands/move_spline.h"
#include "editor/geometry/path.h"
#include "editor/graphics/buffer.h"
#include "editor/graphics/shared_resources.h"
#include "editor/geometry/translation_axes.h"
#include "plant_generator/math/math.h"
#include <QOpenGLFunctions_4_3_Core>
#include <QOpenGLWidget>
#include <QtWidgets>

class CurveEditor : public QOpenGLWidget, protected QOpenGLFunctions_4_3_Core {
	Q_OBJECT

public:
	CurveEditor(
		SharedResources *shared, KeyMap *keymap,
		QWidget *parent = 0);
	QSize sizeHint() const;

public slots:
	void setCurve(pg::Spline spline, QString name);
	void setDegree(int degree);
	void setEnabled(bool enabled);

signals:
	void curveChanged(pg::Spline spline, QString name);
	void editingFinished();

protected:
	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);
	void mousePressEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void mouseMoveEvent(QMouseEvent *);
	void keyPressEvent(QKeyEvent *event);
	void wheelEvent(QWheelEvent *event);

private:
	SharedResources *shared;
	KeyMap *keymap;
	PointSelection selection;
	Camera camera;
	History history;
	Buffer buffer;
	Path path;
	pg::Spline spline;
	pg::Spline origSpline; /* The original spline without restrictions. */
	TranslationAxes axes;
	Command *command = nullptr;
	Geometry::Segment gridSegment;
	Geometry::Segment planeSegment;
	Geometry::Segment controlSegment;
	Geometry::Segment curveSegment;

	QString name;
	bool enabled;
	bool ctrl;
	bool moveLeft;
	pg::Vec3 origPoint;
	int toolBarHeight = 22;

	QComboBox *degree;

	void initiateMovePoint();
	void applyRestrictions();
	void restrictLinearControls();
	void restrictLinearControl(std::vector<pg::Vec3> &controls, int i);
	void restrictOuterCubicControls(std::vector<pg::Vec3> &controls);
	void restrictCubicControls(std::vector<pg::Vec3> &controls);
	void restrictCubicControl(std::vector<pg::Vec3> &controls, int i);
	bool isCenterSelected(std::set<int>::iterator &it);
	void parallelizeTangents();
	void restrictOppositeCubicControls();
	void truncateCubicControl(std::vector<pg::Vec3> &controls, int i);
	void focusOutEvent(QFocusEvent *event);
	void createInterface();
	void paintCurve(pg::Mat4 &vp);
	void extrude();
	void setClickOffset(int x, int y, pg::Vec3 point);
	void exitCommand(bool changed);
	void change();
};

#endif /* CURVE_EDITOR_H */

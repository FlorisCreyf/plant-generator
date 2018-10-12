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

#ifndef CURVE_EDITOR_H
#define CURVE_EDITOR_H

#include "../camera.h"
#include "../point_selection.h"
#include "../commands/move_spline.h"
#include "../geometry/path.h"
#include "../graphics/buffer.h"
#include "../graphics/shared_resources.h"
#include "../geometry/translation_axes.h"
#include "../history/history.h"
#include "plant_generator/math/math.h"
#include "plant_generator/path.h"
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QtWidgets>

class CurveEditor : public QOpenGLWidget, protected QOpenGLFunctions {
	Q_OBJECT

public:
	CurveEditor(SharedResources *shared, QWidget *parent = 0);
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

private:
	SharedResources *shared;
	PointSelection selection;
	Camera camera;
	History history;
	Buffer buffer;
	Path path;
	pg::Spline spline;
	pg::Spline origSpline; /* The original spline without restrictions. */
	TranslationAxes axes;
	MoveSpline moveSpline;
	Geometry::Segment gridSegment;
	Geometry::Segment planeSegment;
	Geometry::Segment controlSegment;
	Geometry::Segment curveSegment;

	QString name;
	bool enabled;
	bool ctrl;
	bool move = false;
	bool extruding = false;
	bool moveLeft;
	pg::Vec3 origPoint;
	int clickOffset[2];

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
	void setClickOffset(int x, int y, pg::Vec3 point);
};

#endif /* CURVE_EDITOR_H */

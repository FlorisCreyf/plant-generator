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

#ifndef CURVE_EDITOR_H
#define CURVE_EDITOR_H

#include "../geometry/path.h"
#include "../graphics/buffer.h"
#include "../graphics/shared_resources.h"
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

protected:
	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);
	void mousePressEvent(QMouseEvent *);
	void mouseDoubleClickEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void mouseMoveEvent(QMouseEvent *);

private:
	SharedResources *shared;
	Buffer buffer;
	Path path;
	pg::Spline spline;
	pg::Vec3 hidden[4];
	Geometry::Segment gridSegment;
	Geometry::Segment planeSegment;
	Geometry::Segment controlSegment;
	Geometry::Segment curveSegment;
	size_t insertIndex;
	size_t point;
	QString name;
	bool enabled;
	int height;
	int width;
	int margin = 20;
	int topMargin = 22;
	float tangentLength;

	QComboBox *degree;

	void createInterface();
	void updateCurve();
	/** Inserts a point into a linear spline. */
	void insertPoint(int, float, float);
	/** Inserts three points into a cubic spline. */
	void insertCurve(int, float, float);
	bool reinsertCurve(float);
	bool omitCurve(float);
	/** Move the control point of a linear curve. */
	void moveControl(float, float);
	/** Move the outer control point of a cubic bezier curve. */
	void moveOuterControl(float, float);
	/** Move the inner control point of a cubic bezier curve. */
	void moveInnerControl(float, float);
	void moveOppositeTangent();
	void setTangentLength();
	/** Move the first and last control points of a cubic spline. */
	void moveTerminalControl(bool, float);
	void toDeviceCoordinates(float &x, float &y, int width, int height);
	void paintInterface();
	void paintCurve(pg::Mat4 &vp);
	pg::Mat4 createVP();
};

#endif /* CURVE_EDITOR_H */

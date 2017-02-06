/* TreeMaker: 3D tree model editor
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

#include "graphics.h"
#include "shared_resources.h"
#include "vector.h"
#include <QtGui/QOpenGLFunctions>
#include <QOpenGLWidget>

using std::vector;

class CurveEditor : public QOpenGLWidget, protected QOpenGLFunctions {
	Q_OBJECT

public:
	CurveEditor(SharedResources *shared, QWidget *parent = 0);
	QSize sizeHint() const;

public slots:
	void setCurve(std::vector<TMvec3> controls, QString name);
	void setEnabled(bool enabled);

signals:
	void curveChanged(vector<TMvec3> controls, QString name);

protected slots:
	void onFloat();

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
	graphics::BufferSet bufferSet;
	graphics::Fragment gridInfo;
	graphics::Fragment planeInfo;
	graphics::Fragment controlInfo;
	graphics::Fragment curveInfo;

	std::vector<TMvec3> controls;
	TMvec3 curve[4];
	size_t insertIndex;
	size_t point;

	GLuint pointTex;

	QString curveName;
	bool enabled;
	int height;
	int width;
	int margin = 20;

	void loadTextures();
	void createInterface();
	void updateCurve();
	void insertCurve(int, float, float);
	bool reinsertCurve(float);
	bool omitCurve(float);
	void placeOuterControl(float, float);
	void placeInnerControl(float, float);
	void placeTerminalControl(bool, float);
	void toDeviceCoordinates(float &x, float &y, int width, int height);
	void paintInterface();
	void paintCurve(TMmat4 &vp);
	TMmat4 createVP();
};

#endif /* CURVE_EDITOR_H */

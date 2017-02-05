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

#ifndef CURVE_BUTTON_H
#define CURVE_BUTTON_H

#include "graphics.h"
#include "shared_resources.h"
#include "vector.h"
#include <QtGui/QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QtWidgets>

class CurveButton : public QOpenGLWidget, protected QOpenGLFunctions {
	Q_OBJECT

public:
	CurveButton(QString name, SharedResources *shared, QWidget *parent = 0);
	void setControls(TMvec3 *controls, int size);
	void setControls(std::vector<TMvec3> controls);
	std::vector<TMvec3> getControls();
	QString getName();
	void select();

signals:
	void selected(CurveButton *);

protected:
	void mousePressEvent(QMouseEvent *);
	void initializeGL();
	void paintGL();
	QSize sizeHint() const;

private:
	SharedResources *shared;
	graphics::BufferSet bufferSet;
	graphics::Fragment curveInfo;
	std::vector<TMvec3> controls;
	QString name;

	void createGeometry();
};

#endif /* CURVE_BUTTON_H */

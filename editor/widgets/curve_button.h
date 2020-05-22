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

#ifndef CURVE_BUTTON_H
#define CURVE_BUTTON_H

#include "editor/geometry/geometry.h"
#include "editor/graphics/buffer.h"
#include "editor/graphics/shared_resources.h"
#include "plant_generator/math/math.h"
#include "plant_generator/spline.h"
#include <QOpenGLFunctions_4_3_Core>
#include <QOpenGLWidget>
#include <QtWidgets>

class CurveButton : public QOpenGLWidget, protected QOpenGLFunctions_4_3_Core {
	Q_OBJECT

	SharedResources *shared;
	pg::Spline spline;
	Geometry geometry;
	Buffer buffer;
	Geometry::Segment segment;
	QString name;
	bool enabled;

public:
	CurveButton(QString name, SharedResources *shared, QWidget *parent = 0);
	void setCurve(const pg::Spline &spline);
	QString getName();
	void select();
	void setEnabled(bool enabled);

signals:
	void selected(CurveButton *);

protected:
	void mousePressEvent(QMouseEvent *);
	void initializeGL();
	void paintGL();
};

#endif /* CURVE_BUTTON_H */

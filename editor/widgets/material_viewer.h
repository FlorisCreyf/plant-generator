/* Plant Genererator
 * Copyright (C) 2019  Floris Creyf
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

#ifndef MATERIAL_VIEWER_H
#define MATERIAL_VIEWER_H

#include "editor/camera.h"
#include "editor/geometry/geometry.h"
#include "editor/graphics/buffer.h"
#include "editor/graphics/shared_resources.h"
#include "plant_generator/material.h"
#include <QOpenGLFunctions_4_3_Core>
#include <QOpenGLWidget>
#include <QtWidgets>

class MaterialViewer :
	public QOpenGLWidget, protected QOpenGLFunctions_4_3_Core {
	Q_OBJECT

	SharedResources *shared;
	Camera camera;
	Buffer buffer;
	Geometry::Segment planeSegment;
	ShaderParams params;

	void createInterface();

protected:
	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent *event);

public:
	MaterialViewer(SharedResources *shared, QWidget *parent);
	QSize sizeHint() const;

public slots:
	void updateMaterial(ShaderParams params);

signals:
	void ready();
};

#endif /* MATERIAL_VIEWER_H */

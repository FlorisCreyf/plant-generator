/* Plant Generator
 * Copyright (C) 2019  Floris Creyf
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

#ifndef MATERIAL_VIEWER_H
#define MATERIAL_VIEWER_H

#include "editor/camera.h"
#include "editor/geometry/geometry.h"
#include "editor/graphics/vertex_buffer.h"
#include "editor/graphics/shared_resources.h"
#include "plant_generator/material.h"
#include <QOpenGLWidget>
#include <QtWidgets>

class MaterialViewer : public QOpenGLWidget,
	protected QOpenGLFunctions_4_3_Core {
	Q_OBJECT

	SharedResources *shared;
	Camera camera;
	VertexBuffer buffer;
	Geometry::Segment planeSegment;
	unsigned materialIndex;

	void createInterface();

protected:
	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);

public:
	MaterialViewer(SharedResources *shared, QWidget *parent);
	QSize sizeHint() const;
	void updateMaterial(unsigned materialIndex);
};

#endif

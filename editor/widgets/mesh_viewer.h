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

#ifndef MESH_VIEWER_H
#define MESH_VIEWER_H

#include "../camera.h"
#include "../graphics/buffer.h"
#include "../graphics/shared_resources.h"
#include "plant_generator/geometry.h"
#include <vector>
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QtWidgets>

class MeshViewer : public QOpenGLWidget, protected QOpenGLFunctions {
	Q_OBJECT

	SharedResources *shared;
	Camera camera;
	Buffer buffer;
	GLsizei count;

protected:
	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);

public:
	MeshViewer(SharedResources *shared, QWidget *parent);
	QSize sizeHint() const;

public slots:
	void updateMesh(pg::Geometry mesh);

signals:
	void ready();
};

#endif /* MESH_VIEWER_H */

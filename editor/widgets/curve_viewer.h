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

 #ifndef CURVE_VIEWER_H
 #define CURVE_VIEWER_H

#include "editor/camera.h"
#include "editor/point_selection.h"
#include "editor/geometry/path.h"
#include "editor/graphics/vertex_buffer.h"
#include "editor/graphics/shared_resources.h"
#include <QtWidgets>

class CurveViewer : public QOpenGLWidget, protected QOpenGLFunctions_4_3_Core {
	Q_OBJECT

	SharedResources *shared;
	Camera camera;
	VertexBuffer buffer;
	Path path;
	Geometry geometry;

	Geometry::Segment gridSegment;
	Geometry::Segment planeSegment;
	Geometry::Segment controlSegment;
	Geometry::Segment curveSegment;

	void appendInterface(Geometry &geometry);

protected:
	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);
	void wheelEvent(QWheelEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

public:
	CurveViewer(SharedResources *shared, QWidget *parent = 0);
	const Camera *getCamera() const;
	void change(const pg::Spline &spline, const PointSelection &selection);
	void clear();

signals:
	void mousePressed(QMouseEvent *event);
	void mouseReleased(QMouseEvent *event);
	void mouseMoved(QMouseEvent *event);
};

#endif /* CURVE_VIEWER_H */

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

#ifndef EDITOR_H
#define EDITOR_H

#define GL_GLEXT_PROTOTYPES

#include "../geometry/path.h"
#include "../geometry/rotation_axes.h"
#include "../geometry/translation_axes.h"
#include "../graphics/buffer.h"
#include "../graphics/shared_resources.h"
#include "../plant.h"
#include "../camera.h"
#include "../history.h"
#include <QtGui/QOpenGLFunctions>
#include <QOpenGLWidget>

class Editor : public QOpenGLWidget, protected QOpenGLFunctions {
	Q_OBJECT

signals:
	void selectionChanged();
	void modeChanged();

public:
	Editor(SharedResources *shared, QWidget *parent = 0);
	void change();
	pg::Plant *getPlant();
	pg::Stem *getSelectedStem();
	void setSelectedStem(pg::Stem *selection);
	int getSelectedPoint();
	void setSelectedPoint(int selection);
	const pg::Mesh *getMesh();
	History *getHistory();
	void revert(History::Memento m);
	bool isExecutingAction();

protected:
	void updateSelection();
	void mousePressEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void keyPressEvent(QKeyEvent *);
	void mouseMoveEvent(QMouseEvent *);
	bool event(QEvent *);

private:
	enum Mode {
		None,
		MovePoint,
		Rotate
	} mode = None;
	struct Scene {
		Geometry::Segment grid;
		Geometry::Segment axesLines;
		Geometry::Segment axesArrows;
		Geometry::Segment selection;
		Geometry::Segment rotation;
	} scene;
	Buffer staticBuffer;
	Buffer pathBuffer;
	Buffer plantBuffer;
	Plant plant;
	pg::Mesh mesh;
	pg::Generator generator;
	Path path;
	pg::Stem *selectedStem = nullptr;
	int selectedPoint = -1;
	SharedResources *shared;
	Camera camera;
	TranslationAxes translationAxes;
	RotationAxes rotationAxes;
	History history;
	int clickOffset[2];

	void extrude();
	void removePoint();
	void addStem(int x, int y);
	void selectStem(int x, int y);
	void selectPoint(int x, int y);
	void selectAxis(int x, int y);
	void movePoint(int x, int y);
	void rotateStem(int x, int y);
	void setClickOffset(int x, int y, pg::Vec3 point);
	void initializeGL();
	void initializeBuffers();
	void paintGL();
	void paintAxes();
	void resizeGL(int width, int height);
};

#endif /* EDITOR_H */

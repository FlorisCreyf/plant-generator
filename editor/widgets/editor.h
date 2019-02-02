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

#include "../camera.h"
#include "../history.h"
#include "../selection.h"
#include "../commands/add_stem.h"
#include "../commands/move_stem.h"
#include "../commands/move_path.h"
#include "../commands/rotate_stem.h"
#include "../commands/save_selection.h"
#include "../geometry/path.h"
#include "../geometry/rotation_axes.h"
#include "../geometry/translation_axes.h"
#include "../graphics/buffer.h"
#include "../graphics/shared_resources.h"
#include "plant_generator/plant.h"
#include "plant_generator/mesh.h"
#include "plant_generator/generator.h"
#include <QtGui/QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QToolBar>
#include <QToolButton>
#include <QWidgetAction>
#include <QComboBox>
#include <set>

class Editor : public QOpenGLWidget, protected QOpenGLFunctions {
	Q_OBJECT

signals:
	void selectionChanged();
	void modeChanged();

public slots:
	void change(QAction *action);
	void updateMaterial(ShaderParams params);

public:
	Editor(SharedResources *shared, QWidget *parent = 0);
	void change();
	void load(const char *filename);
	pg::Plant *getPlant();
	Selection *getSelection();
	const pg::Mesh *getMesh();
	void add(Command &);
	void undo();
	void redo();

protected:
	void updateSelection();
	void mousePressEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void keyPressEvent(QKeyEvent *);
	void mouseMoveEvent(QMouseEvent *);
	bool event(QEvent *);

private:
	QToolBar *toolbar;
	QComboBox *projectionBox;
	QComboBox *shaderBox;
	QAction *perspectiveAction;
	QAction *orthographicAction;
	QAction *wireframeAction;
	QAction *solidAction;
	QAction *materialAction;

	bool perspective;
	Shader shader;

	pg::Plant plant;
	enum Mode {
		None,
		MovePoint,
		PositionStem,
		Rotate
	} mode = None;
	struct Scene {
		Geometry::Segment grid;
		Geometry::Segment axesLines;
		Geometry::Segment axesArrows;
		Geometry::Segment selection;
		Geometry::Segment rotation;
	} scene;
	SharedResources *shared;
	Buffer staticBuffer;
	Buffer pathBuffer;
	Buffer plantBuffer;
	Path path;
	pg::Mesh mesh;
	std::vector<pg::Segment> meshes;
	pg::Generator generator;

	Camera camera;
	TranslationAxes translationAxes;
	RotationAxes rotationAxes;

	Selection selection;
	RotateStem rotateStem;
	MoveStem moveStem;
	MovePath movePath;
	bool extrudeCommand;
	bool addCommand;
	AddStem addStem;
	History history;

	/* An offset is needed because the cursor is not necessarily at the
	 * center of the selection. */
	int clickOffset[2];

	void selectStem(QMouseEvent *event);
	void selectPoint(QMouseEvent *event);
	void selectAxis(int x, int y);
	void setClickOffset(int x, int y, pg::Vec3 point);
	void initializeGL();
	void initializeBuffers();
	void paintGL();
	void paintAxes();
	void resizeGL(int width, int height);
	void updateCamera(int width, int height);
};

#endif /* EDITOR_H */

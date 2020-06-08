/* Plant Generator
 * Copyright (C) 2016-2018  Floris Creyf
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

#ifndef EDITOR_H
#define EDITOR_H

#define PG_SERIALIZE

#include "editor/camera.h"
#include "editor/history.h"
#include "editor/keymap.h"
#include "editor/selection.h"
#include "editor/commands/save_selection.h"
#include "editor/geometry/path.h"
#include "editor/geometry/rotation_axes.h"
#include "editor/geometry/translation_axes.h"
#include "editor/graphics/buffer.h"
#include "editor/graphics/shared_resources.h"

#include "plant_generator/plant.h"
#include "plant_generator/mesh.h"
#include "plant_generator/pseudo_generator.h"
#include "plant_generator/wind.h"

#include <QOpenGLFunctions_4_3_Core>
#include <QOpenGLWidget>
#include <QToolBar>
#include <QToolButton>
#include <QWidgetAction>
#include <QComboBox>

class Editor : public QOpenGLWidget, protected QOpenGLFunctions_4_3_Core {
	Q_OBJECT

signals:
	void selectionChanged();
	void modeChanged();
	void ready();
	void changed();

public slots:
	void change();
	void change(QAction *action);
	void updateMaterial(ShaderParams params);

public:
	Editor(SharedResources *shared, KeyMap *keymap, QWidget *parent = 0);
	void load(const char *filename);
	pg::Plant *getPlant();
	Selection *getSelection();
	const pg::Mesh *getMesh();
	void add(Command *command);
	void undo();
	void redo();

protected:
	void updateSelection();
	void mousePressEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void keyPressEvent(QKeyEvent *);
	void mouseMoveEvent(QMouseEvent *);
	void wheelEvent(QWheelEvent *event);
	bool event(QEvent *);

private:
	QAction *materialAction;
	QAction *orthographicAction;
	QAction *perspectiveAction;
	QAction *solidAction;
	QAction *wireframeAction;
	QComboBox *projectionBox;
	QComboBox *shaderBox;
	QToolBar *toolbar;

	struct Scene {
		Geometry::Segment axesArrows;
		Geometry::Segment axesLines;
		Geometry::Segment grid;
		Geometry::Segment rotation;
		Geometry::Segment selection;
	} scene;

	Command *currentCommand;
	KeyMap *keymap;
	SharedResources *shared;

	Buffer pathBuffer;
	Buffer plantBuffer;
	Buffer staticBuffer;
	GLuint outlineColorMap;
	GLuint outlineFrameBuffer;
	SharedResources::Shader shader;

	pg::PseudoGenerator generator;
	pg::Wind wind;
	pg::Mesh mesh;
	pg::Plant plant;
	std::vector<pg::Segment> meshes;
	Path path;

	Camera camera;
	History history;
	RotationAxes rotationAxes;
	Selection selection;
	TranslationAxes translationAxes;

	bool perspective;
	bool rotating = false;

	void createDefaultPlant();
	void addSelectionToHistory(SaveSelection *selection);
	void createToolBar();
	void createFrameBuffers();
	void exitCommand(bool changed);
	void initializeGL();
	void initializeBuffers();
	void paintGL();
	void paintAxes();
	void resizeGL(int width, int height);
	void selectStem(QMouseEvent *event);
	void selectPoint(QMouseEvent *event);
	void selectAxis(int x, int y);
	void setClickOffset(int x, int y, pg::Vec3 point);
	void updateCamera(int width, int height);
};

#endif /* EDITOR_H */

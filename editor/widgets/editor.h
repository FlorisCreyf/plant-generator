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

#include "editor/camera.h"
#include "editor/history.h"
#include "editor/keymap.h"
#include "editor/selection.h"
#include "editor/commands/save_selection.h"
#include "editor/geometry/path.h"
#include "editor/geometry/rotation_axes.h"
#include "editor/geometry/translation_axes.h"
#include "editor/graphics/storage_buffer.h"
#include "editor/graphics/vertex_buffer.h"
#include "editor/graphics/shared_resources.h"

#include "plant_generator/plant.h"
#include "plant_generator/mesh/generator.h"
#include "plant_generator/pattern_generator.h"
#include "plant_generator/scene.h"
#include "plant_generator/wind.h"

#include <QOpenGLWidget>
#include <QtWidgets>

class Editor : public QOpenGLWidget, protected QOpenGLFunctions_4_3_Core {
	Q_OBJECT

public:
	Editor(SharedResources *shared, KeyMap *keymap, QWidget *parent = 0);
	void load(const char *filename);
	void displayVolume(bool display);
	bool showingVolume() const;
	void setDefaultPlant();
	void change();
	void change(QAction *action);
	void animate();
	void changeWind();
	void reset();
	pg::Plant *getPlant();
	pg::Scene *getScene();
	Selection *getSelection();
	History *getHistory();
	const pg::Mesh *getMesh();
	void undo();
	void redo();

signals:
	void selectionChanged();
	void modeChanged();
	void changed();

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
	QTimer *timer;

	struct Segments {
		Geometry::Segment axesArrows;
		Geometry::Segment axesLines;
		Geometry::Segment grid;
		Geometry::Segment plane;
		Geometry::Segment rotation;
		Geometry::Segment selection;
		Geometry::Segment volume;
	} segments;

	Command *command;
	KeyMap *keymap;
	SharedResources *shared;

	VertexBuffer pathBuffer;
	VertexBuffer volumeBuffer;
	VertexBuffer plantBuffer;
	VertexBuffer staticBuffer;
	StorageBuffer jointBuffer;
	SharedResources::Shader shader;
	GLuint msSilhouetteFramebuffer;
	GLuint msSilhouetteMap;
	GLuint silhouetteFramebuffer;
	GLuint silhouetteMap;
	GLuint shadowFramebuffer;
	GLuint shadowMap;
	int shadowMapSize;

	std::vector<pg::Mesh::Segment> selections;
	pg::Scene scene;
	pg::MeshGenerator meshGenerator;
	const pg::Mesh &mesh;
	Path path;

	Camera camera;
	Camera light;
	History history;
	RotationAxes rotationAxes;
	Selection selection;
	TranslationAxes translationAxes;

	bool updatedLight;
	bool perspective;
	bool rotating;
	int ticks;

	void addSelectionToHistory(SaveSelection *);
	void createToolBar();
	void createFramebuffers(bool);
	void deleteFramebuffers();
	void exitCommand(bool);
	void initializeGL();
	void initializeBuffers();
	void paintGL();
	void paintOutline(const pg::Mat4 &);
	void paintWire(const pg::Mat4 &);
	void paintSolid(const pg::Mat4 &, const pg::Vec3 &);
	void paintMaterial(const pg::Mat4 &, const pg::Vec3 &);
	void paintAxes(const pg::Mat4 &, const pg::Vec3 &);
	void paintVolume(const pg::Mat4 &);
	void updateLight();
	void resizeGL(int, int);
	void selectStem(QMouseEvent *);
	void selectPoint(QMouseEvent *);
	void selectAxis(int, int);
	void setClickOffset(int, int, pg::Vec3);
	void updateCamera(int, int);
	void updateBuffers();
	void updateJoints();
	void startAnimation();
	void endAnimation();
	bool isAnimating();
};

#endif

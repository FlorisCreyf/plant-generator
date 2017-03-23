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

#ifndef SCENE_EDITOR_H
#define SCENE_EDITOR_H

#define GL_GLEXT_PROTOTYPES

#include "axis.h"
#include "graphics.h"
#include "treemaker.h"
#include "shared_resources.h"
#include "camera.h"
#include "math.h"
#include <QtGui/QOpenGLFunctions>
#include <QOpenGLWidget>

class Editor : public QOpenGLWidget, protected QOpenGLFunctions {
	Q_OBJECT

public:
	Editor(SharedResources *shared, QWidget *parent = 0);

	treemaker::Tree *getTree();
	int getSelectedStem();
	void exportObject(const char *filename);
	void change();

signals:
	void selectionChanged(treemaker::Tree &tree, int s);
	void pathChanged();

protected:
	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);
	void mousePressEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void keyPressEvent(QKeyEvent *);
	void keyReleaseEvent(QKeyEvent *);
	void mouseMoveEvent(QMouseEvent *);
	bool event(QEvent *);

private:
	int clickOffset[2];
	bool ctrl = false;
	bool shift = false;
	bool midButton = false;

	graphics::BufferSet bufferSets[3];
	graphics::Fragment gridInfo;
	graphics::Fragment treeInfo;
	graphics::Fragment lineInfo;
	graphics::Fragment selection;
	Axis axis;

	int selectedStem = -1;
	int selectedPoint = -1;

	unsigned maxLines = 100;
	unsigned minLines = 100;

	treemaker::Tree tree;
	SharedResources *shared;
	Camera camera;

	void initializeTree();
	void initializeGrid();
	void intializeLines();
	void updateLines(int branch);
	void selectStem(int x, int y);
	void selectPoint(int x, int y);
	void selectAxis(int x, int y);
	void movePoint(int x, int y);
	void updateSelection();
	void paintSelectionWireframe();
	void paintSelectionLines();
	void paintAxis();
	void expandBuffers();
	void createBuffers();
	void updateBuffers();
};

#endif /* SCENE_EDITOR_H */

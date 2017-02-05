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

#include "graphics.h"
#include "treemaker.h"
#include "shared_resources.h"
#include "camera.h"
#include <QtGui/QOpenGLFunctions>
#include <QOpenGLWidget>

class Editor : public QOpenGLWidget, protected QOpenGLFunctions {
	Q_OBJECT

public:
	Editor(SharedResources *shared, QWidget *parent = 0);
	~Editor();

	void exportObject(const char *filename);

public slots:
	void changeResolution(int i);
	void changeSections(int i);
	void changeRadius(double d);
	void changeRadiusCurve(std::vector<TMvec3> c);
	void changeBranchCurve(std::vector<TMvec3> c);
	void changeBranchDensity(double d);

signals:
	void selectionChanged(TMtree tree, int s);

protected:
	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);
	void mousePressEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void keyPressEvent(QKeyEvent *);
	void keyReleaseEvent(QKeyEvent *);
	void mouseMoveEvent(QMouseEvent *);

private:
	bool ctrl;
	bool shift;
	bool midButton;

	graphics::BufferSet bufferSets[2];
	graphics::Fragment gridInfo;
	graphics::Fragment treeInfo;
	graphics::Fragment selection;
	int selectedBranch;

	std::vector<float> vertices;
	std::vector<unsigned short> indices;

	TMtree tree;
	SharedResources *shared;
	Camera camera;

	void initializeTree();
	void initializeGrid();
	void selectBranch(int x, int y);
	void updateSelection();
	void paintSelection();
	void expandBuffers();
	void createBuffers();
	void updateBuffers();
	void change();
};

#endif /* SCENE_EDITOR_H */

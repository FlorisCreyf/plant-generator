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

#ifndef OPENGL_EDITOR_H
#define OPENGL_EDITOR_H

#define GL_GLEXT_PROTOTYPES

#include "treemaker.h"
#include "graphics.h"
#include "shared_resources.h"
#include "camera.h"
#include "axis.h"
#include <QtGui/QOpenGLFunctions>
#include <QOpenGLWidget>

class OpenGLEditor : public QOpenGLWidget, protected QOpenGLFunctions {
	Q_OBJECT
	
public:
	OpenGLEditor(SharedResources *shared, QWidget *parent = 0);
	void change();
	
protected:
	graphics::BufferSet bufferSets[3];
	graphics::Fragment gridInfo;
	graphics::Fragment treeInfo;
	graphics::Fragment lineInfo;
	graphics::Fragment selection;
	Axis axis;

	treemaker::Tree tree;
	SharedResources *shared;
	Camera camera;
	int selectedStem = -1;
	int selectedPoint = -1;
	
	void initializeTree();
	virtual void configureTree() = 0;
	void updateLines(int stem);
	void updateSelection();
	
private:
	unsigned maxLines = 100;
	unsigned minLines = 100;
	
	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);
	void initializeGrid();
	void intializeLines();
	/** This method assumes that the proper vertex array is already set. */
	void paintSelectionWireframe();
	/** This method assumes that FLAT_SHADER is already set. */
	void paintSelectionLines();
	void paintAxis();
	void expandBuffers();
	void createBuffers();
	void updateBuffers();
};

#endif /* OPENGL_EDITOR_H */



/* TreeMaker: 3D tree model editor
 * Copyright (C) 2017  Floris Creyf
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

#ifndef tEDITOR_H
#define tEDITOR_H

#include "opengl_editor.h"

class Editor : public OpenGLEditor {
	Q_OBJECT
	
public:	
	Editor(SharedResources *shared, QWidget *parent = 0);
	
	treemaker::Tree *getTree();
	int getSelectedStem();
	void exportObject(const char *filename);

signals:
	void selectionChanged(treemaker::Tree &tree, int s);
	void modeChanged();
	
protected:
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
	
	void configureTree() override;
	float closestDistance(unsigned stem, int x, int y);
	void extrude();
	void removePoint();
	void addStem();
	void selectStem(int x, int y);
	void selectPoint(int x, int y);
	void selectAxis(int x, int y);
	void movePoint(int x, int y);
	void setClickOffset(int x, int y, treemaker::Vec3 point);
};

#endif /* EDITOR_H */


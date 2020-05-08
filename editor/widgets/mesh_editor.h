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

#ifndef MESH_EDITOR_H
#define MESH_EDITOR_H

#include "mesh_viewer.h"
#include "editor.h"
#include <QtWidgets>

class MeshEditor : public QWidget {
	Q_OBJECT

	SharedResources *shared;
	Editor *editor;
	MeshViewer *meshViewer;
	QComboBox *meshBox;
	pg::Geometry mesh;

	QLineEdit *meshField;
	QPushButton *meshList;
	QMenu *meshMenu;

	QPushButton *addButton;
	QPushButton *removeButton;

	QPushButton *customButton;
	QPushButton *planeButton;
	QPushButton *perpPlaneButton;
	QPushButton *emptyButton;

	QString selectedText;

	void initFields(QVBoxLayout *form);
	void initTopRow(QHBoxLayout *topRow);

public:
	MeshEditor(SharedResources *shared, Editor *editor, QWidget *parent);
	QSize sizeHint() const;
	void clear();
	const MeshViewer *getViewer() const;

public slots:
	void addMesh();
	void addMesh(pg::Geometry);
	void loadCustom();
	void loadPlane();
	void loadPerpPlane();
	void loadEmpty();
	void selectMesh();
	void renameMesh();
	void removeMesh();

signals:
	void meshModified();
	void meshChanged(pg::Geometry mesh);
	void meshAdded(pg::Geometry geom);
	void meshRenamed(QString before, QString after);
	void meshRemoved(QString name);
};

#endif /* MESH_EDITOR_H */

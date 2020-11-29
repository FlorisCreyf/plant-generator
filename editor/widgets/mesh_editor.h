/* Plant Generator
 * Copyright (C) 2019  Floris Creyf
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

#ifndef MESH_EDITOR_H
#define MESH_EDITOR_H

#include "mesh_viewer.h"
#include "item_delegate.h"
#include "editor.h"
#include <QtWidgets>

class MeshEditor : public QWidget {
	Q_OBJECT

	SharedResources *shared;
	Editor *editor;
	MeshViewer *meshViewer;
	pg::Geometry mesh;

	QLineEdit *meshField;
	QPushButton *meshList;
	QMenu *meshMenu;
	QVBoxLayout *layout;
	QComboBox *selectionBox;

	void createSelectionBar();
	void createFields(QVBoxLayout *form);
	void modify(pg::Geometry &, unsigned);

public:
	MeshEditor(SharedResources *shared, Editor *editor, QWidget *parent);
	void reset();
	void clear();
	const MeshViewer *getViewer() const;
	QSize sizeHint() const;

public slots:
	void addEmpty();
	void add(pg::Geometry);
	void select();
	void rename();
	void remove();
	void loadCustom();
	void loadPlane();
	void loadPerpPlane();
	void loadEmpty();

signals:
	void meshAdded(pg::Geometry geom);
	void meshModified(pg::Geometry geom, unsigned index);
	void meshRemoved(unsigned index);
};

#endif

/* Plant Generator
 * Copyright (C) 2018  Floris Creyf
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

#ifndef MATERIAL_EDITOR_H
#define MATERIAL_EDITOR_H

#include "editor.h"
#include "material_viewer.h"
#include <QPushButton>
#include <vector>
#include <QtWidgets>

class MaterialEditor : public QWidget {
	Q_OBJECT

	Editor *editor;
	MaterialViewer *materialViewer;
	SharedResources *shared;

	QVBoxLayout *layout;
	QComboBox *selectionBox;
	QLineEdit *diffuseBox;
	QPushButton *addDiffuseButton;
	QPushButton *removeDiffuseButton;

	void initFields(QFormLayout *form);
	void createSelectionBar();
	void update(ShaderParams params, unsigned index);

public:
	MaterialEditor(SharedResources *shared, Editor *editor,
		QWidget *parent);
	void add(pg::Material material);
	void reset();
	void clear();
	const MaterialViewer *getViewer() const;
	QSize sizeHint() const;

public slots:
	void add();
	void select();
	void rename();
	void remove();
	void openDiffuseFile();
	void removeDiffuseFile();
};

#endif /* MATERIAL_EDITOR_H */

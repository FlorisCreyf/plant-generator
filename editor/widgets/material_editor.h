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

#include "object_editor.h"
#include "editor.h"
#include "material_viewer.h"
#include <QPushButton>
#include <vector>

class MaterialEditor : public ObjectEditor {
	Q_OBJECT

	SharedResources *shared;
	Editor *editor;
	MaterialViewer *materialViewer;

	QLineEdit *diffuseBox;
	QPushButton *addDiffuseButton;
	QPushButton *removeDiffuseButton;

	void initFields(QFormLayout *form);
	void update(ShaderParams params, unsigned index);

public:
	MaterialEditor(
		SharedResources *shared, Editor *editor, QWidget *parent = 0);
	void add(pg::Material material);
	void init(const std::vector<pg::Material> &materials);
	void clear();
	QSize sizeHint() const;
	const MaterialViewer *getViewer() const;

public slots:
	void add();
	void select();
	void rename();
	void remove();
	void openDiffuseFile();
	void removeDiffuseFile();
};

#endif /* MATERIAL_EDITOR_H */

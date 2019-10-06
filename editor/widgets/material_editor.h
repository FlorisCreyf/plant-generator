/* Plant Genererator
 * Copyright (C) 2018  Floris Creyf
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

#ifndef MATERIAL_EDITOR_H
#define MATERIAL_EDITOR_H

#include "material_viewer.h"
#include <QPushButton>
#include <QtWidgets>
#include <map>

class MaterialEditor : public QWidget {
	Q_OBJECT

	SharedResources *shared;
	MaterialViewer *materialViewer;
	QComboBox *materialBox;

	QLineEdit *materialField;
	QPushButton *materialList;
	QMenu *materialMenu;

	QPushButton *addButton;
	QPushButton *removeButton;
	QLineEdit *diffuseBox;
	QPushButton *addDiffuseButton;
	QPushButton *removeDiffuseButton;

	QToolBar *toolbar;

	void initFields(QFormLayout *form);
	void initTopRow(QHBoxLayout *topRow);

public:
	MaterialEditor(SharedResources *shared, QWidget *parent = 0);
	void addMaterial(pg::Material material);
	void clear();
	QSize sizeHint() const;
	const MaterialViewer *getViewer() const;

public slots:
	void addMaterial();
	void openDiffuseFile();
	void removeDiffuseFile();
	void selectMaterial();
	void renameMaterial(const QString &text);
	void removeMaterial();

signals:
	void materialModified(ShaderParams params);
	void materialChanged(ShaderParams params);
	void materialAdded(ShaderParams params);
};

#endif /* MATERIAL_EDITOR_H */

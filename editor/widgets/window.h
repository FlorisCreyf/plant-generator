/* Plant Genererator
 * Copyright (C) 2016-2018  Floris Creyf
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

#ifndef WINDOW_H
#define WINDOW_H

#include "../qt/ui_window.h"
#include "../graphics/shared_resources.h"
#include "editor.h"
#include "curve_editor.h"
#include "material_editor.h"
#include "mesh_editor.h"
#include "property_box.h"

class Window : public QMainWindow {
	Q_OBJECT

public:
	Window();

public Q_SLOTS:
	void newFile();
	void openDialogBox();
	void exportDialogBox();
	void saveAsDialogBox();
	void saveDialogBox();
	void reportIssue();

private:
	Ui::Window widget;
	SharedResources shared;
	Editor *editor;
	PropertyBox *propertyBox;
	CurveEditor *curveEditor;
	MaterialEditor *materialEditor;
	MeshEditor *meshEditor;
	QString filename;

	void keyPressEvent(QKeyEvent *event);
	void createPropertyBox();
	void createEditors();
};

#endif /* WINDOW_H */

/* Plant Generator
 * Copyright (C) 2016-2018  Floris Creyf
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

#ifndef WINDOW_H
#define WINDOW_H

#include "generator_editor.h"
#include "generator_curve_editor.h"
#include "property_curve_editor.h"
#include "material_editor.h"
#include "mesh_editor.h"
#include "wind_editor.h"
#include "key_editor.h"
#include "property_editor.h"
#include "editor.h"

#include "editor/keymap.h"
#include "editor/graphics/shared_resources.h"
#include "editor/qt/ui_window.h"

#include <QtWidgets>

class Window : public QMainWindow {
	Q_OBJECT

public:
	Window(int argc, char **argv);

public Q_SLOTS:
	void newFile();
	void openDialogBox();
	void exportWavefrontDialogBox();
	void exportColladaDialogBox();
	void saveAsDialogBox();
	void saveDialogBox();
	void reportIssue();
	void initEditor();
	void updateStatus();

private:
	Ui::Window widget;
	SharedResources shared;
	KeyMap keymap;
	Editor *editor;
	QString filename;
	QLabel *objectLabel;
	QLabel *fileLabel;

	PropertyEditor *propertyEditor;
	GeneratorCurveEditor *gcEditor;
	PropertyCurveEditor *pcEditor;
	GeneratorEditor *generatorEditor;
	WindEditor *windEditor;
	MaterialEditor *materialEditor;
	MeshEditor *meshEditor;
	KeyEditor *keyEditor;

	void keyPressEvent(QKeyEvent *event);
	void createPropertyBox();
	void createEditors();
	QWidget *createGeneratorEditor();
	QDockWidget *createDockWidget(const char *, QWidget *, bool);
	void setFilename(QString filename);
};

#endif

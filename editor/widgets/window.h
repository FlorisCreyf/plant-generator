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
#define PG_SERIALIZE

#ifndef VIEWPORT_ONLY
#include "generator_curve_editor.h"
#include "property_curve_editor.h"
#include "procedural_editor.h"
#include "material_editor.h"
#include "mesh_editor.h"
#include "key_editor.h"
#include "property_editor.h"
#endif

#include "editor/keymap.h"
#include "editor/graphics/shared_resources.h"
#include "editor/qt/ui_window.h"
#include "editor.h"

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

	#ifndef VIEWPORT_ONLY
	PropertyEditor *propertyEditor;
	GeneratorCurveEditor *gCurveEditor;
	PropertyCurveEditor *pCurveEditor;
	ProceduralEditor *procEditor;
	MaterialEditor *materialEditor;
	MeshEditor *meshEditor;
	KeyEditor *keyEditor;
	#endif

	void keyPressEvent(QKeyEvent *event);
	void createPropertyBox();
	void createEditors();
	QDockWidget *createDockWidget(const char *, QWidget *, bool);
	void setFilename(QString filename);
};

#endif /* WINDOW_H */

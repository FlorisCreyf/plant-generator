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

#include "editor.h"
#include "generator_editor.h"
#include "pattern_editor.h"
#include "property_editor.h"
#include "key_editor.h"
#include "editor/keymap.h"
#include "editor/graphics/shared_resources.h"
#include "qt/ui_window.h"
#include <QtWidgets>

class Window : public QMainWindow {
	Q_OBJECT

public:
	Window(int argc, char **argv);

public slots:
	void newEmptyFile();
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
	GeneratorEditor *generatorEditor;
	PatternEditor *patternEditor;
	KeyEditor *keyEditor;

	void keyPressEvent(QKeyEvent *event);
	void createPropertyBox();
	void createMenu();
	void createEditors();
	QDockWidget *createDW(const char *, QWidget *, bool);
	void setFilename(QString filename);

	template<typename T>
	void addAction(const char *name, QMenu *menu, QKeySequence keySequence,
		T callback)
	{
		auto action = new QAction(name, this);
		action->setShortcut(keySequence);
		menu->addAction(action);
		connect(action, &QAction::triggered, this, callback);
	}
};

#endif

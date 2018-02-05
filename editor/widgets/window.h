/* Plant Genererator
 * Copyright (C) 2016-2017  Floris Creyf
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

#ifndef WINDOW_H
#define WINDOW_H

#include "../qt/ui_window.h"
#include "../graphics/shared_resources.h"
#include "editor.h"
#include "curve_editor.h"
#include "property_box.h"

class Window : public QMainWindow {
	Q_OBJECT

public:
	Window();

public Q_SLOTS:
	void openDialogBox();
	void exportDialogBox();
	void saveDialogBox();
	void reportIssue();

private:
	Ui::Window widget;
	SharedResources shared;
	Editor *editor;
	PropertyBox *propertyBox;
	CurveEditor *curveEditor;

	void createPropertyBox();
	void createCurveEditor();
};

#endif /* WINDOW_H */

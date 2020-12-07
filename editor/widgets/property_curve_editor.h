/* Plant Generator
 * Copyright (C) 2020  Floris Creyf
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

#ifndef PROPERTY_CURVE_EDITOR_H
#define PROPERTY_CURVE_EDITOR_H

#include "curve_editor.h"
#include "widgets.h"

class PropertyCurveEditor : public CurveEditor {
	Q_OBJECT

	Editor *editor;
	ComboBox *selectionBox;

	void change(bool curveChanged);
	void createSelectionBar();

public:
	PropertyCurveEditor(SharedResources *shared, KeyMap *keymap,
		Editor *editor, QWidget *parent);
	void reset();
	void clear();

public slots:
	void add();
	void add(pg::Curve curve);
	void select();
	void rename();
	void remove();

signals:
	void curveAdded(pg::Curve curve);
	void curveModified(pg::Curve curve, unsigned index);
	void curveRemoved(unsigned index);
	void editingFinished();
};

#endif

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

#ifndef GENERATOR_EDITOR_H
#define GENERATOR_EDITOR_H

#include "form.h"
#include "editor.h"

class GeneratorEditor : public Form {
	Q_OBJECT

	Editor *editor;
	QDoubleSpinBox *stemDensity;
	QDoubleSpinBox *stemStart;
	QDoubleSpinBox *leafDensity;
	QDoubleSpinBox *leafStart;
	QPushButton *createButton;

	void createInterface();
	void removeCurrent();
	QSize sizeHint() const;

public:
	GeneratorEditor(Editor *editor, QWidget *parent);

public slots:
	void submit();
};

#endif

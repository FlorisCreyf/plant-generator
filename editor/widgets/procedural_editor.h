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

#ifndef PROCEDURAL_EDITOR_H
#define PROCEDURAL_EDITOR_H

#include "editor.h"
#include "generator_editor.h"
#include "wind_editor.h"
#include <QtWidgets>

class ProceduralEditor : public QWidget {
	Q_OBJECT

	GeneratorEditor *genEditor;
	WindEditor *windEditor;

public:
	ProceduralEditor(Editor *editor, QWidget *parent);
	const GeneratorEditor *getGeneratorEditor() const;
	const WindEditor *getWindEditor() const;
	QSize sizeHint() const;
};

#endif

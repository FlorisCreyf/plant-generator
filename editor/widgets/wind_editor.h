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

#ifndef WIND_EDITOR_H
#define WIND_EDITOR_H

#include "editor.h"
#include "widgets.h"
#include "plant_generator/wind.h"

class WindEditor : public QWidget {
	Q_OBJECT

	Editor *editor;

	enum {DirectionX, DirectionY, DirectionZ, DSize};
	enum {TimeStep, FrameCount, Seed, ISize};

	QGroupBox *group;
	DoubleSpinBox *dv[DSize];
	SpinBox *iv[ISize];

	void createInterface();
	void blockSignals(bool);

public:
	WindEditor(Editor *editor, QWidget *parent);
	QSize sizeHint() const;
	void setFields();

public slots:
	void change();
};

#endif

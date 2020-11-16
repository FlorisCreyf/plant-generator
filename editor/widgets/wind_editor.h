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
#include "plant_generator/wind.h"
#include <QGroupBox>
#include <QDoubleSpinBox>
#include <QSpinBox>

class WindEditor : public QWidget {
	Q_OBJECT

	Editor *editor;

	QGroupBox *group;
	QDoubleSpinBox *directionXValue;
	QDoubleSpinBox *directionYValue;
	QDoubleSpinBox *directionZValue;
	QSpinBox *timeStepValue;
	QSpinBox *frameCountValue;
	QSpinBox *seedValue;

	void createInterface();

public:
	WindEditor(Editor *editor, QWidget *parent);
	QSize sizeHint() const;

public slots:
	void change();
};

#endif /* WIND_EDITOR_H */

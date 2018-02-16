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

#ifndef PROPERTY_BOX
#define PROPERTY_BOX

#include "editor.h"
#include "curve_button.h"
#include "curve_editor.h"
#include <QtWidgets>

class PropertyBox : public QWidget {
	Q_OBJECT

public:
	PropertyBox(SharedResources *shared, QWidget *parent = 0);
	void bind(Editor *editor, CurveEditor *curveEditor);
	QSize sizeHint() const;

public slots:
	void fill();
	void setCurve(pg::Spline spline, QString names);
	void toggleCurve(CurveButton *w);
	void changePathDegree(int i);
	void changeResolution(int i);
	void changeDivisions(int i);
	void changeRadius(double d);
	void changeRadiusCurve(pg::Spline &spline);
	void finishChanging();

signals:
	void isEnabled(bool enabled);
	void radiusCurveChanged(std::vector<pg::Vec3>);

private:
	SharedResources *shared;
	CurveButton *activeCurve;
	CurveEditor *curveEditor;
	Editor *editor;

	QTableWidget *global;
	QTableWidget *local;
	QGroupBox *localGroup;
	QDoubleSpinBox *radius;
	CurveButton *radiusCB;
	QSpinBox *resolution;
	QSpinBox *divisions;
	QComboBox *degree;
	bool changing = false;

	void beginChanging(QWidget *widget);
	void createLocalBox(QVBoxLayout *layout);
	QWidget *createCenteredWidget(QWidget *);
	void configureTable(QTableWidget *);
};

#endif /* PROPERTY_BOX */
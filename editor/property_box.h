/* TreeMaker: 3D tree model editor
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
	QSize sizeHint() const;
	void bind(Editor *editor, CurveEditor *curveEditor);

public slots:
	void fill(treemaker::Tree &tree, int stem);
	void setCurve(std::vector<treemaker::Vec3> control, QString names);
	void toggleCurve(CurveButton *w);

signals:
	void isEnabled(bool enabled);
	void radiusCurveChanged(std::vector<treemaker::Vec3>);

private:
	SharedResources *shared;
	Editor *editor;
	CurveEditor *curveEditor;
	CurveButton *activeCurve;

	QTableWidget *global;
	QDoubleSpinBox *crownBaseHeight;
	QDoubleSpinBox *apicalDominance;

	QTableWidget *local;
	QGroupBox *localGroup;
	QDoubleSpinBox *radius;
	CurveButton *radiusCB;
	QSpinBox *resolution;
	QSpinBox *sections;
	QDoubleSpinBox *stems;
	QDoubleSpinBox *base;

	void createGlobalBox(QVBoxLayout *layout);
	void createLocalBox(QVBoxLayout *layout);
	void bindCurveEditor();
	void fillCurveButtons(treemaker::Tree &tree, int stem);
	QWidget *createCenteredWidget(QWidget *);
	void configureTable(QTableWidget *);
};

#endif /* PROPERTY_BOX */

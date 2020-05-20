/* Plant Genererator
 * Copyright (C) 2020  Floris Creyf
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

#ifndef LEAF_EDITOR_H
#define LEAF_EDITOR_H

#include "form.h"

class LeafEditor : public Form {
	Q_OBJECT

public:
	LeafEditor(SharedResources *shared, Editor *editor, QWidget *parent);
	void setFields(std::map<pg::Stem *, std::set<long>> instances);
	bool addMaterial(ShaderParams params);
	long removeMaterial(QString name);
	void renameMaterial(QString before, QString after);
	void addMesh(pg::Geometry geom);
	void renameMesh(QString before, QString after);
	bool removeMesh(QString name);
	QSize sizeHint() const;

public slots:
	void changeXScale(double d);
	void changeYScale(double d);
	void changeZScale(double d);
	void changeLeafMaterial();
	void changeLeafMesh();

signals:
	void changed();

private:
	SharedResources *shared;
	Editor *editor;

	QGroupBox *leafGroup;
	QLabel *scaleXLabel;
	QDoubleSpinBox *scaleXValue;
	QLabel *scaleYLabel;
	QDoubleSpinBox *scaleYValue;
	QLabel *scaleZLabel;
	QDoubleSpinBox *scaleZValue;
	QLabel *materialLabel;
	QComboBox *materialValue;
	QLabel *meshLabel;
	QComboBox *meshValue;

	void blockSignals(bool);
	void createInterface();
	void enable(bool);
};

#endif

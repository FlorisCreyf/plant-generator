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

#ifndef LEAF_EDITOR_H
#define LEAF_EDITOR_H

#include "form.h"

class LeafEditor : public Form {
	Q_OBJECT

public:
	LeafEditor(SharedResources *shared, Editor *editor, QWidget *parent);
	void setFields(std::map<pg::Stem *, std::set<size_t>> instances);
	bool addMaterial(ShaderParams params);
	void removeMaterial(unsigned index);
	void updateMaterials();
	void addMesh(pg::Geometry geom);
	void updateMesh(pg::Geometry geom, unsigned index);
	void removeMesh(unsigned);
	void clearOptions();
	QSize sizeHint() const;

public slots:
	void finishChanging();
	void changeCustom(int custom);
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

	SaveStem *saveStem;

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
	QLabel *customLabel;
	QCheckBox *customValue;

	void blockSignals(bool);
	void createInterface();
	void enable(bool);
	void beginChanging(QLabel *label);
};

#endif

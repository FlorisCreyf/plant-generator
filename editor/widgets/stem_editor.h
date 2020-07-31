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

#ifndef STEM_EDITOR_H
#define STEM_EDITOR_H

#include "form.h"
#include "curve_editor.h"
#include <map>

class StemEditor : public Form {
	Q_OBJECT

public:
	StemEditor(SharedResources *shared, Editor *editor, QWidget *parent);
	void setFields(std::map<pg::Stem *, PointSelection> instances);
	bool addCurve(pg::Curve curve);
	void updateCurve(pg::Curve curve, unsigned index);
	void removeCurve(unsigned index);
	bool addMaterial(ShaderParams params);
	void updateMaterials();
	void removeMaterial(unsigned index);
	QSize sizeHint() const;

public slots:
	void finishChanging();
	void changeCustom(int custom);
	void changePathDegree(int degree);
	void changeSectionDivisions(int divisions);
	void changeCollarDivisions(int divisions);
	void changePathDivisions(int divisions);
	void changeRadius(double radius);
	void changeMinRadius(double minRadius);
	void changeRadiusCurve(int curve);
	void changeStemMaterial();
	void changeXCollar(double x);
	void changeYCollar(double y);
	void changeCapMaterial();

private:
	SharedResources *shared;
	Editor *editor;
	SaveStem *saveStem;

	QGroupBox *stemGroup;
	QLabel *radiusLabel;
	QDoubleSpinBox *radiusValue;
	QLabel *minRadiusLabel;
	QDoubleSpinBox *minRadiusValue;
	QLabel *radiusCurveLabel;
	QComboBox *radiusCurveValue;
	QLabel *sDivisionLabel;
	QSpinBox *sDivisionValue;
	QLabel *cDivisionLabel;
	QSpinBox *cDivisionValue;
	QLabel *pDivisionLabel;
	QSpinBox *pDivisionValue;
	QLabel *degreeLabel;
	QComboBox *degreeValue;
	QLabel *stemMaterialLabel;
	QComboBox *stemMaterialValue;
	QLabel *collarXLabel;
	QDoubleSpinBox *collarXValue;
	QLabel *collarYLabel;
	QDoubleSpinBox *collarYValue;
	QLabel *capMaterialLabel;
	QComboBox *capMaterialValue;
	QLabel *customLabel;
	QCheckBox *customValue;

	void blockSignals(bool);
	void createInterface();
	void enable(bool);
	void beginChanging(QLabel *label);
};

#endif

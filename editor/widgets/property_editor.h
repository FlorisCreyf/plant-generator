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

#ifndef PROPERTY_EDITOR
#define PROPERTY_EDITOR

#include "editor.h"
#include "curve_button.h"
#include "curve_editor.h"
#include "editor/commands/save_stem.h"
#include <QtWidgets>

class PropertyEditor : public QWidget {
	Q_OBJECT

public:
	PropertyEditor(SharedResources *shared, Editor *editor, QWidget *parent);
	void bind(CurveEditor *curveEditor);
	QSize sizeHint() const;

public slots:
	void fill();
	void changePathDegree(int i);
	void changeResolution(int i );
	void changeDivisions(int i);
	void changeRadius(double d);
	void changeMinRadius(double d);
	void changeRadiusCurve(pg::Spline &spline);
	void changeStemMaterial();
	void changeXCollar(double d);
	void changeYCollar(double d);
	void changeCapMaterial();
	void changeXScale(double d);
	void changeYScale(double d);
	void changeZScale(double d);
	void changeLeafMaterial();
	void changeLeafMesh();
	void finishChanging();
	void setCurve(pg::Spline spline, QString names);
	void toggleCurve(CurveButton *w);
	void addMaterial(ShaderParams params);
	void renameMaterial(QString before, QString after);
	void removeMaterial(QString name);
	void addMesh(pg::Geometry geom);
	void renameMesh(QString before, QString after);
	void removeMesh(QString name);

signals:
	void isEnabled(bool enabled);
	void radiusCurveChanged(std::vector<pg::Vec3>);

private:
	SharedResources *shared;
	Editor *editor;
	SaveStem *saveStem;
	CurveEditor *curveEditor;
	CurveButton *selectedCurve;

	QGroupBox *stemG;
	QLabel *radiusL;
	QDoubleSpinBox *radiusV;
	QLabel *minRadiusL;
	QDoubleSpinBox *minRadiusV;
	CurveButton *radiusB;
	QLabel *resolutionL;
	QSpinBox *resolutionV;
	QLabel *divisionL;
	QSpinBox *divisionV;
	QLabel *degreeL;
	QComboBox *degreeV;
	QLabel *stemMaterialL;
	QComboBox *stemMaterialV;
	QLabel *collarXL;
	QDoubleSpinBox *collarXV;
	QLabel *collarYL;
	QDoubleSpinBox *collarYV;

	QGroupBox *capG;
	QLabel *capMaterialL;
	QComboBox *capMaterialV;

	QGroupBox *leafG;
	QLabel *leafScaleXL;
	QDoubleSpinBox *leafScaleXV;
	QLabel *leafScaleYL;
	QDoubleSpinBox *leafScaleYV;
	QLabel *leafScaleZL;
	QDoubleSpinBox *leafScaleZV;
	QLabel *leafMaterialL;
	QComboBox *leafMaterialV;
	QLabel *leafMeshL;
	QComboBox *leafMeshV;

	bool changing = false;

	void beginChanging();
	void createStemBox(QVBoxLayout *);
	void createLeafBox(QVBoxLayout *);
	void createCapBox(QVBoxLayout *);
	void blockStemSignals(bool);
	void blockLeafSignals(bool);
	void enableStem(bool);
	void enableLeaf(bool);
	void indicateDifferences(QWidget *);
	void indicateSimilarities(QWidget *);
	void initProperties();
	void setLeafFields(std::map<pg::Stem *, std::set<long>>);
	void setStemFields(std::map<pg::Stem *, PointSelection>);
	void setValueWidths(QFormLayout *);
};

#endif /* PROPERTY_EDITOR */
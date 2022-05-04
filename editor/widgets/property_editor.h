/* Plant Generator
 * Copyright (C) 2016-2018  Floris Creyf
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

#ifndef PROPERTY_EDITOR_H
#define PROPERTY_EDITOR_H

#include "curve_editor.h"
#include "mesh_viewer.h"
#include "material_viewer.h"
#include "editor.h"
#include "widgets.h"
#include "editor/keymap.h"
#include "../commands/save_stem.h"
#include "../commands/save_curve.h"

class PropertyEditor : public QWidget {
	Q_OBJECT

	Editor *editor;
	KeyMap *keymap;
	SharedResources *shared;
	SaveStem *saveStem;
	SaveCurve *saveCurve;
	bool sameAsCurrent;

	enum {SectionDivisions, PathDivisions, CollarDivisions, ISize};
	enum {Radius, MinRadius, CollarX, CollarY, ScaleX, ScaleY, ScaleZ,
		DSize};
	enum {RadiusCurve, PathDegree, StemMaterial, CapMaterial, LeafMaterial,
		LeafMesh, CSize};
	enum {CustomStem, CustomLeaf, BSize};

	SpinBox *iv[ISize];
	QLabel *il[ISize];
	DoubleSpinBox *dv[DSize];
	QLabel *dl[DSize];
	ComboBox *cv[CSize];
	QLabel *cl[CSize];
	QCheckBox *bv[BSize];
	QLabel *bl[BSize];

	void blockSignals(bool);
	void createInterface();
	void createStemLeafFields();
	void createStemInterface(QBoxLayout *);
	void createLeafInterface(QBoxLayout *);
	void setStemFields(const std::map<pg::Stem *, PointSelection> &);
	void setStemValues(const pg::Stem *stem);
	void setLeafFields(const std::map<pg::Stem *, std::set<size_t>> &);
	void setLeafValues(const pg::Leaf *leaf);
	void enableStemFields(bool);
	void enableLeafFields(bool);
	void beginChanging(QLabel *);

	template<class T, class U>
	void changeLeaf(const T &func, U value, QLabel *label)
	{
		beginChanging(label);
		auto a = this->editor->getSelection()->getLeafInstances();
		for (auto &b : a)
			for (size_t index : b.second)
				func(b.first->getLeaf(index), value);
		this->sameAsCurrent = this->saveStem->isSameAsCurrent();
		this->editor->change();
	}

	template<class T, class U>
	void changeStem(const T &func, U value, QLabel *label)
	{
		beginChanging(label);
		auto a = this->editor->getSelection()->getStemInstances();
		for (auto &b : a)
			func(b.first, value);
		this->sameAsCurrent = this->saveStem->isSameAsCurrent();
		this->editor->change();
	}

	CurveEditor *curveEditor;
	ComboBox *curveName;
	ComboBox *curveDegree;

	void createCurveInterface(QBoxLayout *);
	void setCurveDegree(int);
	void setCurves();
	void addCurve();
	void addCurve(pg::Curve);
	void addCurveName(QString);
	void selectCurve();
	void renameCurve();
	void removeCurve();
	void clearCurves();
	void changeCurve(bool);
	void createCurveCommand();

	enum {Shininess, AmbientR, AmbientG, AmbientB, MSize};

	MaterialViewer *materialViewer;
	ComboBox *materialName;
	QLineEdit *materialFile[pg::Material::MapQuantity];
	DoubleSpinBox *materialValue[MSize];

	void createMaterialInterface(QBoxLayout *);
	void openMaterialFile(int);
	void removeMaterialFile(int);
	void selectMaterial(int);
	void updateMaterial(ShaderParams, unsigned);
	void changeMaterial();
	void renameMaterial();
	void addMaterial();
	void addMaterialName(QString);
	void removeMaterial();

	MeshViewer *meshViewer;
	ComboBox *meshName;

	void createMeshInterface(QBoxLayout *);
	void addMesh();
	void addMeshName(QString);
	void removeMesh();
	void selectMesh();
	void renameMesh();
	void loadCustomMesh();
	void loadPlaneMesh();
	void loadPerpPlaneMesh();
	void loadEmptyMesh();
	void modifyMesh(pg::Geometry &, int);

	enum {DirectionX, DirectionY, DirectionZ, Threshold, Resistance,
		WDSize};
	enum {TimeStep, FrameCount, Seed, WISize};

	DoubleSpinBox *wdv[DSize];
	SpinBox *wiv[ISize];

	void createWindInterface(QBoxLayout *);
	void changeWind();
	void setWindFields();
	void blockWindSignals(bool);

public:
	PropertyEditor(SharedResources *shared, KeyMap *keymap, Editor *editor,
		QWidget *parent);
	void populate();
	QSize sizeHint() const;
	void setFields();
	void clear();
	void finishChanging();
};

#endif

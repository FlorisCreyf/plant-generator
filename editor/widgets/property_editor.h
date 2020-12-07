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

#include "editor.h"
#include "widgets.h"
#include "../commands/save_stem.h"

class PropertyEditor : public QWidget {
	Q_OBJECT

	Editor *editor;
	SharedResources *shared;
	SaveStem *saveStem;
	bool sameAsCurrent;

	enum {SectionDivisions, PathDivisions, CollarDivisions, ISize};
	enum {Radius, MinRadius, CollarX, CollarY, ScaleX, ScaleY, ScaleZ,
		DSize};
	enum {RadiusCurve, Degree, StemMaterial, CapMaterial, LeafMaterial,
		Mesh, CSize};
	enum {CustomStem, CustomLeaf, BSize};

	QGroupBox *stemGroup;
	QGroupBox *leafGroup;
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
		this->editor->change();
	}

	template<class T, class U>
	void changeStem(const T &func, U value, QLabel *label)
	{
		beginChanging(label);
		auto a = this->editor->getSelection()->getStemInstances();
		for (auto &b : a)
			func(b.first, value);
		this->editor->change();
	}

public:
	PropertyEditor(SharedResources *shared, Editor *editor,
		QWidget *parent);
	QSize sizeHint() const;

public slots:
	void setFields();
	void clearOptions();
	void finishChanging();

	void addCurve(pg::Curve curve);
	void updateCurve(pg::Curve curve, int index);
	void removeCurve(int index);
	void addMaterial(ShaderParams params);
	void updateMaterials();
	void removeMaterial(int index);
	void addMesh(pg::Geometry geom);
	void updateMesh(pg::Geometry geom, int index);
	void removeMesh(int index);
};

#endif

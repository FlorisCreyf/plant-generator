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

#include "property_box.h"
#include <iterator>

using pg::Spline;
using pg::Stem;
using pg::Vec3;
using pg::Leaf;
using pg::Plant;

using std::next;
using std::prev;
using std::string;
using std::map;
using std::set;

PropertyBox::PropertyBox(
	SharedResources *shared, Editor *editor, QWidget *parent) :
	QWidget(parent)
{
	this->shared = shared;
	this->curveEditor = nullptr;
	this->selectedCurve = nullptr;
	this->changing = false;

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setSizeConstraint(QLayout::SetMinimumSize);
	layout->setSpacing(0);
	layout->setMargin(0);
	createStemBox(layout);
	createCapBox(layout);
	createLeafBox(layout);
	enableStem(false);
	layout->addStretch(1);

	this->leafMaterialV->setEnabled(false);

	this->editor = editor;
	connect(this->editor, SIGNAL(selectionChanged()), this, SLOT(fill()));
}

void PropertyBox::setValueWidths(QFormLayout *layout)
{
	layout->setFormAlignment(Qt::AlignRight | Qt::AlignTop);
	layout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
	layout->setLabelAlignment(Qt::AlignRight);
	for(int i = 0; i < layout->rowCount(); i++) {
		QLayoutItem *item = layout->itemAt(i, QFormLayout::FieldRole);
		if (item && item->widget())
			item->widget()->setFixedWidth(250);
	}
}

void PropertyBox::createStemBox(QVBoxLayout *layout)
{
	this->stemG = new QGroupBox(tr("Stem"));
	QFormLayout *form = new QFormLayout(this->stemG);
	form->setSpacing(2);
	form->setMargin(10);

	{
		QWidget *sizeWidget = new QWidget();
		QHBoxLayout *line = new QHBoxLayout();
		sizeWidget->setLayout(line);
		this->radiusL = new QLabel(tr("Radius"));
		this->radiusV = new QDoubleSpinBox;
		this->radiusB = new CurveButton("Radius", this->shared, this);
		this->radiusB->setFixedWidth(22);
		this->radiusB->setFixedHeight(22);
		this->radiusV->setSingleStep(0.001);
		this->radiusV->setDecimals(3);
		line->addWidget(this->radiusV);
		line->addWidget(this->radiusB);
		line->setSpacing(0);
		line->setMargin(0);
		form->addRow(this->radiusL, sizeWidget);
	}

	this->minRadiusL = new QLabel(tr("Min Radius"));
	this->minRadiusV = new QDoubleSpinBox;
	this->minRadiusV->setSingleStep(0.001);
	this->minRadiusV->setDecimals(3);
	form->addRow(this->minRadiusL, this->minRadiusV);

	this->divisionL = new QLabel(tr("Divisions"));
	this->divisionV = new QSpinBox;
	this->divisionV->setMinimum(1);
	form->addRow(this->divisionL, this->divisionV);

	this->resolutionL = new QLabel(tr("Resolution"));
	this->resolutionV = new QSpinBox;
	this->resolutionV->setMinimum(5);
	form->addRow(this->resolutionL, this->resolutionV);

	this->degreeL = new QLabel(tr("Degree"));
	this->degreeV = new QComboBox;
	this->degreeV->addItem(QString("Linear"));
	this->degreeV->addItem(QString("Cubic"));
	form->addRow(this->degreeL, this->degreeV);

	this->stemMaterialL = new QLabel(tr("Material"));
	this->stemMaterialV = new QComboBox;
	this->stemMaterialV->addItem(tr(""), QVariant(0));
	form->addRow(this->stemMaterialL, this->stemMaterialV);

	setValueWidths(form);
	this->stemG->setSizePolicy(
		QSizePolicy::Expanding, QSizePolicy::Minimum);
	layout->addWidget(this->stemG);

	connect(this->degreeV, SIGNAL(currentIndexChanged(int)),
		this, SLOT(changePathDegree(int)));
	connect(this->resolutionV, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
	connect(this->resolutionV, SIGNAL(valueChanged(int)),
		this, SLOT(changeResolution(int)));
	connect(this->divisionV, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
	connect(this->divisionV, SIGNAL(valueChanged(int)),
		this, SLOT(changeDivisions(int)));
	connect(this->radiusV, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
	connect(this->radiusV, SIGNAL(valueChanged(double)),
		this, SLOT(changeRadius(double)));
	connect(this->minRadiusV, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
	connect(this->minRadiusV, SIGNAL(valueChanged(double)),
		this, SLOT(changeMinRadius(double)));
	connect(this->radiusB, SIGNAL(selected(CurveButton *)),
		this, SLOT(toggleCurve(CurveButton *)));
	connect(this->stemMaterialV, SIGNAL(currentIndexChanged(int)),
		this, SLOT(changeStemMaterial()));
}

void PropertyBox::createCapBox(QVBoxLayout *layout)
{
	this->capG = new QGroupBox(tr("Cap"));
	QFormLayout *form = new QFormLayout(capG);
	form->setSpacing(2);
	form->setMargin(10);

	this->capMaterialL = new QLabel(tr("Material"));
	this->capMaterialV = new QComboBox;
	this->capMaterialV->addItem(tr(""), QVariant(0));
	form->addRow(this->capMaterialL, this->capMaterialV);

	setValueWidths(form);
	this->capG->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	layout->addWidget(this->capG);

	connect(this->capMaterialV, SIGNAL(currentIndexChanged(int)),
		this, SLOT(changeCapMaterial()));
}

void PropertyBox::createLeafBox(QVBoxLayout *layout)
{
	this->leafG = new QGroupBox(tr("Leaf"));
	QFormLayout *form = new QFormLayout(this->leafG);
	form->setSpacing(2);
	form->setMargin(10);

	this->leafScaleXL = new QLabel(tr("X Scale"));
	this->leafScaleXV = new QDoubleSpinBox;
	this->leafScaleXV->setMinimum(0.01);
	this->leafScaleXV->setSingleStep(0.1);
	form->addRow(this->leafScaleXL, this->leafScaleXV);

	this->leafScaleYL = new QLabel(tr("Y Scale"));
	this->leafScaleYV = new QDoubleSpinBox;
	this->leafScaleYV->setMinimum(0.01);
	this->leafScaleYV->setSingleStep(0.1);
	form->addRow(this->leafScaleYL, this->leafScaleYV);

	this->leafScaleZL = new QLabel(tr("Z Scale"));
	this->leafScaleZV = new QDoubleSpinBox;
	this->leafScaleZV->setMinimum(0.01);
	this->leafScaleZV->setSingleStep(0.1);
	form->addRow(this->leafScaleZL, this->leafScaleZV);

	this->leafMaterialL = new QLabel(tr("Material"));
	this->leafMaterialV = new QComboBox;
	this->leafMaterialV->addItem(tr(""), QVariant(0));
	form->addRow(this->leafMaterialL, this->leafMaterialV);

	this->leafMeshL = new QLabel(tr("Mesh"));
	this->leafMeshV = new QComboBox;
	this->leafMeshV->addItem(tr(""), QVariant(0));
	form->addRow(this->leafMeshL, this->leafMeshV);

	enableLeaf(false);
	setValueWidths(form);
	this->leafG->setSizePolicy(
		QSizePolicy::Expanding, QSizePolicy::Minimum);
	layout->addWidget(this->leafG);

	connect(this->leafScaleXV, SIGNAL(valueChanged(double)),
		this, SLOT(changeXScale(double)));
	connect(this->leafScaleXV, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
	connect(this->leafScaleYV, SIGNAL(valueChanged(double)),
		this, SLOT(changeYScale(double)));
	connect(this->leafScaleYV, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
	connect(this->leafScaleZV, SIGNAL(valueChanged(double)),
		this, SLOT(changeZScale(double)));
	connect(this->leafScaleZV, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
	connect(this->leafMaterialV, SIGNAL(currentIndexChanged(int)),
		this, SLOT(changeLeafMaterial()));
	connect(this->leafMeshV, SIGNAL(currentIndexChanged(int)),
		this, SLOT(changeLeafMesh()));
}

void PropertyBox::fill()
{
	auto leafInstances = this->editor->getSelection()->getLeafInstances();
	auto stemInstances = this->editor->getSelection()->getStemInstances();

	if (stemInstances.empty()) {
		enableStem(false);
		this->curveEditor->setEnabled(false);
	} else
		setStemFields(stemInstances);

	if (leafInstances.empty())
		enableLeaf(false);
	else
		setLeafFields(leafInstances);

	if (this->selectedCurve)
		toggleCurve(this->selectedCurve);
}

void PropertyBox::setStemFields(map<Stem *, PointSelection> instances)
{
	Stem *stem = instances.rbegin()->first;
	auto nextIt = next(instances.begin());
	blockStemSignals(true);

	indicateSimilarities(this->resolutionL);
	for (auto it = nextIt; it != instances.end(); ++it) {
		Stem *a = prev(it)->first;
		Stem *b = it->first;
		if (a->getResolution() != b->getResolution()) {
			indicateDifferences(this->resolutionL);
			break;
		}
	}

	this->resolutionV->setValue(stem->getResolution());

	indicateSimilarities(this->divisionL);
	for (auto it = nextIt; it != instances.end(); ++it) {
		pg::Path a = prev(it)->first->getPath();
		pg::Path b = it->first->getPath();
		if (a.getResolution() != b.getResolution()) {
			indicateDifferences(this->divisionL);
			break;
		}
	}

	this->divisionV->setValue(stem->getPath().getResolution());

	indicateSimilarities(this->radiusL);
	for (auto it = nextIt; it != instances.end(); ++it) {
		pg::Path a = prev(it)->first->getPath();
		pg::Path b = it->first->getPath();
		if (a.getMaxRadius() != b.getMaxRadius()) {
			indicateDifferences(this->radiusL);
			break;
		}
	}

	this->radiusV->setValue(stem->getPath().getMaxRadius());
	this->radiusB->setCurve(stem->getPath().getRadius());

	indicateSimilarities(this->minRadiusL);
	for (auto it = nextIt; it != instances.end(); ++it) {
		pg::Path a = prev(it)->first->getPath();
		pg::Path b = it->first->getPath();
		if (a.getMinRadius() != b.getMinRadius()) {
			indicateDifferences(this->minRadiusL);
			break;
		}
	}

	this->minRadiusV->setValue(stem->getPath().getMinRadius());

	indicateSimilarities(this->degreeL);
	for (auto it = nextIt; it != instances.end(); ++it) {
		Spline a = prev(it)->first->getPath().getSpline();
		Spline b = it->first->getPath().getSpline();
		if (a.getDegree() != b.getDegree()) {
			indicateDifferences(this->degreeL);
			break;
		}
	}

	switch (stem->getPath().getSpline().getDegree()) {
		case 1:
			this->degreeV->setCurrentIndex(0);
			break;
		case 3:
			this->degreeV->setCurrentIndex(1);
			break;
	}

	indicateSimilarities(this->stemMaterialL);
	for (auto it = nextIt; it != instances.end(); ++it) {
		int a = prev(it)->first->getMaterial(Stem::Outer);
		int b = it->first->getMaterial(Stem::Outer);
		if (a != b) {
			indicateDifferences(this->stemMaterialL);
			break;
		}
	}

	{
		int id = stem->getMaterial(Stem::Outer);
		string s = this->shared->getMaterial(id).getName();
		QString qs = QString::fromStdString(s);
		this->stemMaterialV->setCurrentText(qs);
	}

	indicateSimilarities(this->capMaterialL);
	for (auto it = nextIt; it != instances.end(); ++it) {
		int a = prev(it)->first->getMaterial(Stem::Inner);
		int b = it->first->getMaterial(Stem::Inner);
		if (a != b) {
			indicateDifferences(this->capMaterialL);
			break;
		}
	}

	{
		int id = stem->getMaterial(Stem::Inner);
		string s = shared->getMaterial(id).getName();
		QString qs = QString::fromStdString(s);
		this->capMaterialV->setCurrentText(qs);
	}

	if (this->selectedCurve)
		this->selectedCurve->select();

	enableStem(true);
	this->curveEditor->setEnabled(true);
	blockStemSignals(false);
}

void PropertyBox::setLeafFields(map<Stem *, set<long>> instances)
{
	Stem *stem = instances.rbegin()->first;
	long leafID = *instances.rbegin()->second.begin();
	Leaf *leaf = stem->getLeaf(leafID);
	blockLeafSignals(true);

	indicateSimilarities(this->leafScaleXL);
	indicateSimilarities(this->leafScaleYL);
	indicateSimilarities(this->leafScaleZL);
	for (auto it = instances.begin(); it != instances.end(); it++) {
		Stem *stem = it->first;
		for (auto &id : it->second) {
			Vec3 scale = stem->getLeaf(id)->getScale();
			if (scale.x != leaf->getScale().x)
				indicateDifferences(this->leafScaleXL);
			if (scale.y != leaf->getScale().y)
				indicateDifferences(this->leafScaleYL);
			if (scale.z != leaf->getScale().z)
				indicateDifferences(this->leafScaleZL);
		}
	}

	this->leafScaleXV->setValue(leaf->getScale().x);
	this->leafScaleYV->setValue(leaf->getScale().y);
	this->leafScaleZV->setValue(leaf->getScale().z);

	indicateSimilarities(this->leafMaterialL);
	indicateSimilarities(this->leafMeshL);
	for (auto it = instances.begin(); it != instances.end(); it++) {
		Stem *stem = it->first;
		for (auto &id : it->second) {
			Leaf *l = stem->getLeaf(id);
			long material = l->getMaterial();
			long mesh = l->getMesh();
			if (material != leaf->getMaterial())
				indicateDifferences(this->leafMaterialL);
			if (mesh != leaf->getMesh())
				indicateDifferences(this->leafMeshL);
		}
	}

	{
		long id = leaf->getMaterial();
		string s = this->shared->getMaterial(id).getName();
		QString qs = QString::fromStdString(s);
		this->leafMaterialV->setCurrentText(qs);
	}

	if (leaf->getMesh() != 0) {
		long id = leaf->getMesh();
		Plant *plant = this->editor->getPlant();
		pg::Geometry geom = plant->getLeafMesh(id);
		QString qs = QString::fromStdString(geom.getName());
		this->leafMeshV->setCurrentText(qs);
	} else
		this->leafMeshV->setCurrentText(tr(""));

	blockLeafSignals(false);
	enableLeaf(true);
}

void PropertyBox::addMaterial(ShaderParams params)
{
	QString name;
	name = QString::fromStdString(params.getName());
	if (this->stemMaterialV->findText(name) < 0) {
		qlonglong id = params.getID();
		this->stemMaterialV->addItem(name, QVariant(id));
		this->capMaterialV->addItem(name, QVariant(id));
		this->leafMaterialV->addItem(name, QVariant(id));
		this->editor->getPlant()->addMaterial(params.getMaterial());
	}
	editor->change();
}

void PropertyBox::removeMaterial(QString name)
{
	int index = this->stemMaterialV->findText(name);
	if (index != 0) {
		long id = this->stemMaterialV->itemData(index).toInt();
		this->editor->getPlant()->removeMaterial(id);
		this->stemMaterialV->removeItem(index);
		index = this->capMaterialV->findText(name);
		this->capMaterialV->removeItem(index);
		index = this->leafMaterialV->findText(name);
		this->leafMaterialV->removeItem(index);
		this->editor->change();
	}
}

void PropertyBox::renameMaterial(QString before, QString after)
{
	int index;
	index = this->stemMaterialV->findText(before);
	this->stemMaterialV->setItemText(index, after);
	index = this->capMaterialV->findText(before);
	this->capMaterialV->setItemText(index, after);
	index = this->leafMaterialV->findText(before);
	this->leafMaterialV->setItemText(index, after);
}

void PropertyBox::addMesh(pg::Geometry geom)
{
	QString name = QString::fromStdString(geom.getName());
	if (leafMeshV->findText(name) < 0) {
		qlonglong id = geom.getID();
		this->leafMeshV->addItem(name, QVariant(id));
	}
	this->editor->change();
}

void PropertyBox::renameMesh(QString before, QString after)
{
	int index = this->leafMeshV->findText(before);
	this->leafMeshV->setItemText(index, after);
}

void PropertyBox::removeMesh(QString name)
{
	int index = this->leafMeshV->findText(name);
	if (index != 0) {
		this->leafMeshV->removeItem(index);
		this->editor->change();
	}
}

void PropertyBox::changePathDegree(int i)
{
	beginChanging();
	indicateSimilarities(this->degreeL);
	int degree = i == 1 ? 3 : 1;
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances) {
		pg::Path path = instance.first->getPath();
		Spline spline = path.getSpline();
		if (spline.getDegree() != degree) {
			spline.adjust(degree);
			this->editor->getSelection()->clearPoints();
		}
		path.setSpline(spline);
		instance.first->setPath(path);
	}
	this->editor->getSelection()->clearPoints();
	this->editor->change();
	finishChanging();
}

void PropertyBox::changeResolution(int i)
{
	beginChanging();
	indicateSimilarities(this->resolutionL);
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances)
		instance.first->setResolution(i);
	this->editor->change();
}

void PropertyBox::changeDivisions(int i)
{
	beginChanging();
	indicateSimilarities(this->divisionL);
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances) {
		pg::Path path = instance.first->getPath();
		path.setResolution(i);
		instance.first->setPath(path);
	}
	this->editor->change();
}

void PropertyBox::changeRadius(double d)
{
	beginChanging();
	indicateSimilarities(this->radiusL);
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances) {
		pg::Path path = instance.first->getPath();
		path.setMaxRadius(d);
		instance.first->setPath(path);
	}
	this->editor->change();
}

void PropertyBox::changeMinRadius(double d)
{
	beginChanging();
	indicateSimilarities(this->minRadiusL);
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances) {
		pg::Path path = instance.first->getPath();
		path.setMinRadius(d);
		instance.first->setPath(path);
	}
	this->editor->change();
}

void PropertyBox::changeRadiusCurve(pg::Spline &spline)
{
	beginChanging();
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances) {
		pg::Path vp = instance.first->getPath();
		vp.setRadius(spline);
		instance.first->setPath(vp);
	}
	this->editor->change();
}

void PropertyBox::changeStemMaterial()
{
	beginChanging();
	indicateSimilarities(this->stemMaterialL);
	int id = this->stemMaterialV->currentData().toInt();
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances)
		instance.first->setMaterial(Stem::Outer, id);
	this->editor->change();
	finishChanging();
}

void PropertyBox::changeCapMaterial()
{
	beginChanging();
	indicateSimilarities(this->capMaterialL);
	int id = this->capMaterialV->currentData().toInt();
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances)
		instance.first->setMaterial(Stem::Inner, id);
	this->editor->change();
	finishChanging();
}

void PropertyBox::changeXScale(double d)
{
	beginChanging();
	indicateSimilarities(this->leafScaleXL);
	auto instances = this->editor->getSelection()->getLeafInstances();
	for (auto &instance : instances) {
		Stem *stem = instance.first;
		for (auto &leaf : instance.second) {
			Vec3 scale = stem->getLeaf(leaf)->getScale();
			scale.x = d;
			stem->getLeaf(leaf)->setScale(scale);
		}
	}
	this->editor->change();
}

void PropertyBox::changeYScale(double d)
{
	beginChanging();
	indicateSimilarities(this->leafScaleYL);
	auto instances = this->editor->getSelection()->getLeafInstances();
	for (auto &instance : instances) {
		Stem *stem = instance.first;
		for (auto &leaf : instance.second) {
			Vec3 scale = stem->getLeaf(leaf)->getScale();
			scale.y = d;
			stem->getLeaf(leaf)->setScale(scale);
		}
	}
	this->editor->change();
}

void PropertyBox::changeZScale(double d)
{
	beginChanging();
	indicateSimilarities(this->leafScaleYL);
	auto instances = this->editor->getSelection()->getLeafInstances();
	for (auto &instance : instances) {
		Stem *stem = instance.first;
		for (auto &leaf : instance.second) {
			Vec3 scale = stem->getLeaf(leaf)->getScale();
			scale.z = d;
			stem->getLeaf(leaf)->setScale(scale);
		}
	}
	this->editor->change();
}

void PropertyBox::changeLeafMaterial()
{
	beginChanging();
	indicateSimilarities(this->leafMaterialL);
	int id = this->leafMaterialV->currentData().toInt();
	auto instances = this->editor->getSelection()->getLeafInstances();
	for (auto &instance : instances) {
		Stem *stem = instance.first;
		for (long leaf : instance.second)
			stem->getLeaf(leaf)->setMaterial(id);
	}
	this->editor->change();
	finishChanging();
}

void PropertyBox::changeLeafMesh()
{
	beginChanging();
	indicateSimilarities(this->leafMeshL);
	int id = this->leafMeshV->currentData().toInt();
	auto instances = this->editor->getSelection()->getLeafInstances();
	for (auto &instance : instances) {
		Stem *stem = instance.first;
		for (auto &leaf : instance.second)
			stem->getLeaf(leaf)->setMesh(id);
	}
	this->editor->change();
	finishChanging();
}

void PropertyBox::beginChanging()
{
	if (!this->changing) {
		this->saveStem = new SaveStem(this->editor->getSelection());
		this->saveStem->execute();
	}
	this->changing = true;
}

void PropertyBox::finishChanging()
{
	if (this->changing && !this->saveStem->isSameAsCurrent()) {
		this->saveStem->setNewSelection();
		this->editor->add(*this->saveStem);
	}
	this->changing = false;
}

void PropertyBox::blockStemSignals(bool block)
{
	this->resolutionV->blockSignals(block);
	this->divisionV->blockSignals(block);
	this->radiusV->blockSignals(block);
	this->radiusB->blockSignals(block);
	this->minRadiusV->blockSignals(block);
	this->degreeV->blockSignals(block);
	this->stemMaterialV->blockSignals(block);
	this->capMaterialV->blockSignals(block);
	this->stemG->blockSignals(block);
}

void PropertyBox::blockLeafSignals(bool block)
{
	this->leafScaleXV->blockSignals(block);
	this->leafScaleYV->blockSignals(block);
	this->leafScaleZV->blockSignals(block);
	this->leafMaterialV->blockSignals(block);
	this->leafMeshV->blockSignals(block);
}

void PropertyBox::enableStem(bool enable)
{
	if (!enable) {
		indicateSimilarities(this->radiusL);
		indicateSimilarities(this->minRadiusL);
		indicateSimilarities(this->resolutionL);
		indicateSimilarities(this->divisionL);
		indicateSimilarities(this->degreeL);
		indicateSimilarities(this->stemMaterialL);
		indicateSimilarities(this->capMaterialL);
	}

	this->radiusV->setEnabled(enable);
	this->radiusB->setEnabled(enable);
	this->minRadiusV->setEnabled(enable);
	this->resolutionV->setEnabled(enable);
	this->divisionV->setEnabled(enable);
	this->degreeV->setEnabled(enable);
	this->stemMaterialV->setEnabled(enable);
	this->capMaterialV->setEnabled(enable);
}

void PropertyBox::enableLeaf(bool enable)
{
	if (!enable) {
		indicateSimilarities(this->leafScaleXV);
		indicateSimilarities(this->leafScaleYV);
		indicateSimilarities(this->leafScaleZV);
		indicateSimilarities(this->leafMaterialV);
		indicateSimilarities(this->leafMeshV);
	}

	this->leafScaleXV->setEnabled(enable);
	this->leafScaleYV->setEnabled(enable);
	this->leafScaleZV->setEnabled(enable);
	this->leafMaterialV->setEnabled(enable);
	this->leafMeshV->setEnabled(enable);
}

void PropertyBox::indicateDifferences(QWidget *widget)
{
	widget->setStyleSheet("font-weight:bold;");
}

void PropertyBox::indicateSimilarities(QWidget *widget)
{
	widget->setStyleSheet("");
}

void PropertyBox::bind(CurveEditor *curveEditor)
{
	this->curveEditor = curveEditor;
	connect(curveEditor, SIGNAL(curveChanged(pg::Spline, QString)),
		this, SLOT(setCurve(pg::Spline, QString)));
	connect(curveEditor, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
}

QSize PropertyBox::sizeHint() const
{
	return QSize(350, 200);
}

void PropertyBox::setCurve(pg::Spline spline, QString name)
{
	if (name == "Radius") {
		this->radiusB->setCurve(spline);
		changeRadiusCurve(spline);
	}
}

void PropertyBox::toggleCurve(CurveButton *button)
{
	this->selectedCurve = button;
	auto instances = this->editor->getSelection()->getStemInstances();
	if (!instances.empty()) {
		Stem *stem = instances.rbegin()->first;
		QString name = button->getName();
		this->curveEditor->setCurve(stem->getPath().getRadius(), name);
	}
}

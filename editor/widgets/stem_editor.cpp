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

#include "stem_editor.h"
#include <iterator>
#include <string>

using pg::Stem;
using pg::Spline;
using pg::Vec2;
using pg::Vec3;
using std::string;
using std::next;
using std::prev;
using std::map;

StemEditor::StemEditor(
	SharedResources *shared, Editor *editor, QWidget *parent) :
	Form(parent)
{
	this->shared = shared;
	this->editor = editor;
	this->saveStem = nullptr;
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	createInterface();
	enable(false);
}

QSize StemEditor::sizeHint() const
{
	return this->stemGroup->sizeHint();
}

void StemEditor::createInterface()
{
	this->stemGroup = new QGroupBox(tr("Stem"), this);
	this->stemGroup->setSizePolicy(
		QSizePolicy::Expanding, QSizePolicy::Minimum);

	QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
	layout->setMargin(0);
	layout->setSpacing(0);
	layout->addWidget(this->stemGroup);

	QFormLayout *form = new QFormLayout(this->stemGroup);
	form->setSpacing(2);
	form->setMargin(5);

	this->radiusLabel = new QLabel(tr("Radius"));
	this->radiusValue = new QDoubleSpinBox;
	this->radiusValue->setSingleStep(0.001);
	this->radiusValue->setDecimals(3);
	form->addRow(this->radiusLabel, this->radiusValue);

	this->minRadiusLabel = new QLabel(tr("Min Radius"));
	this->minRadiusValue = new QDoubleSpinBox;
	this->minRadiusValue->setSingleStep(0.001);
	this->minRadiusValue->setDecimals(3);
	form->addRow(this->minRadiusLabel, this->minRadiusValue);

	this->radiusCurveLabel = new QLabel(tr("Radius Curve"));
	this->radiusCurveValue = new QComboBox;
	form->addRow(this->radiusCurveLabel, this->radiusCurveValue);

	this->divisionLabel = new QLabel(tr("Divisions"));
	this->divisionValue = new QSpinBox;
	this->divisionValue->setMinimum(1);
	form->addRow(this->divisionLabel, this->divisionValue);

	this->resolutionLabel = new QLabel(tr("Resolution"));
	this->resolutionValue = new QSpinBox;
	this->resolutionValue->setMinimum(3);
	form->addRow(this->resolutionLabel, this->resolutionValue);

	this->degreeLabel = new QLabel(tr("Degree"));
	this->degreeValue = new QComboBox;
	this->degreeValue->addItem(QString("Linear"));
	this->degreeValue->addItem(QString("Cubic"));
	form->addRow(this->degreeLabel, this->degreeValue);

	this->stemMaterialLabel = new QLabel(tr("Material"));
	this->stemMaterialValue = new QComboBox;
	form->addRow(this->stemMaterialLabel, this->stemMaterialValue);

	this->capMaterialLabel = new QLabel(tr("Cap Material"));
	this->capMaterialValue = new QComboBox;
	form->addRow(this->capMaterialLabel, this->capMaterialValue);

	this->collarXLabel = new QLabel(tr("Collar (X)"));
	this->collarXValue = new QDoubleSpinBox;
	this->collarXValue->setSingleStep(0.1);
	this->collarXValue->setDecimals(3);
	form->addRow(this->collarXLabel, this->collarXValue);

	this->collarYLabel = new QLabel(tr("Collar (Y)"));
	this->collarYValue = new QDoubleSpinBox;
	this->collarYValue->setSingleStep(0.1);
	this->collarYValue->setDecimals(3);
	form->addRow(this->collarYLabel, this->collarYValue);

	this->customLabel = new QLabel(tr("Custom"));
	this->customValue = new QCheckBox;
	form->addRow(this->customLabel, this->customValue);

	setValueWidths(form);

	connect(this->customValue, SIGNAL(stateChanged(int)),
		this, SLOT(changeCustom(int)));
	connect(this->degreeValue, SIGNAL(currentIndexChanged(int)),
		this, SLOT(changePathDegree(int)));
	connect(this->resolutionValue, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
	connect(this->resolutionValue, SIGNAL(valueChanged(int)),
		this, SLOT(changeResolution(int)));
	connect(this->divisionValue, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
	connect(this->divisionValue, SIGNAL(valueChanged(int)),
		this, SLOT(changeDivisions(int)));
	connect(this->radiusValue, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
	connect(this->radiusValue, SIGNAL(valueChanged(double)),
		this, SLOT(changeRadius(double)));
	connect(this->minRadiusValue, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
	connect(this->minRadiusValue, SIGNAL(valueChanged(double)),
		this, SLOT(changeMinRadius(double)));
	connect(this->radiusCurveValue, SIGNAL(currentIndexChanged(int)),
		this, SLOT(changeRadiusCurve(int)));
	connect(this->stemMaterialValue, SIGNAL(currentIndexChanged(int)),
		this, SLOT(changeStemMaterial()));
	connect(this->collarXValue, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
	connect(this->collarXValue, SIGNAL(valueChanged(double)),
		this, SLOT(changeXCollar(double)));
	connect(this->collarYValue, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
	connect(this->collarYValue, SIGNAL(valueChanged(double)),
		this, SLOT(changeYCollar(double)));
	connect(this->capMaterialValue, SIGNAL(currentIndexChanged(int)),
		this, SLOT(changeCapMaterial()));
}

void StemEditor::setFields(map<Stem *, PointSelection> instances)
{
	enable(false);
	if (instances.empty())
		return;

	Stem *stem = instances.rbegin()->first;
	auto nextIt = next(instances.begin());
	blockSignals(true);

	for (auto it = nextIt; it != instances.end(); ++it) {
		Stem *stem1 = prev(it)->first;
		Stem *stem2 = it->first;
		if (stem1->isCustom() != stem2->isCustom())
			indicateDifferences(this->customLabel);
		if (stem1->getResolution() != stem2->getResolution())
			indicateDifferences(this->resolutionLabel);
		if (stem1->getMaxRadius() != stem2->getMaxRadius())
			indicateDifferences(this->radiusLabel);
		if (stem1->getMinRadius() != stem2->getMinRadius())
			indicateDifferences(this->minRadiusLabel);
		if (stem1->getRadiusCurve() != stem2->getRadiusCurve())
			indicateDifferences(this->radiusCurveLabel);

		pg::Path path1 = prev(it)->first->getPath();
		pg::Path path2 = it->first->getPath();
		if (path1.getResolution() != path2.getResolution())
			indicateDifferences(this->divisionLabel);

		Spline spline1 = path1.getSpline();
		Spline spline2 = path2.getSpline();
		if (spline1.getDegree() != spline2.getDegree())
			indicateDifferences(this->degreeLabel);

		unsigned material1 = stem1->getMaterial(Stem::Outer);
		unsigned material2 = stem2->getMaterial(Stem::Outer);
		if (material1 != material2)
			indicateDifferences(this->stemMaterialLabel);
		material1 = stem1->getMaterial(Stem::Inner);
		material2 = stem2->getMaterial(Stem::Inner);
		if (material1 != material2)
			indicateDifferences(this->capMaterialLabel);

		Vec2 collar1 = stem1->getSwelling();
		Vec2 collar2 = stem2->getSwelling();
		if (collar1.x != collar2.x)
			indicateDifferences(this->collarXLabel);
		if (collar1.y != collar2.y)
			indicateDifferences(this->collarYLabel);
	}

	this->customValue->setCheckState(
		stem->isCustom() ? Qt::Checked : Qt::Unchecked);
	this->resolutionValue->setValue(stem->getResolution());
	this->divisionValue->setValue(stem->getPath().getResolution());
	this->radiusValue->setValue(stem->getMaxRadius());
	this->minRadiusValue->setValue(stem->getMinRadius());
	this->radiusCurveValue->setCurrentIndex(stem->getRadiusCurve());
	if (stem->getPath().getSpline().getDegree() == 3)
		this->degreeValue->setCurrentIndex(1);
	else
		this->degreeValue->setCurrentIndex(0);
	{
		unsigned index = stem->getMaterial(Stem::Outer);
		string s = this->shared->getMaterial(index).getName();
		QString qs = QString::fromStdString(s);
		this->stemMaterialValue->setCurrentText(qs);
		index = stem->getMaterial(Stem::Inner);
		s = shared->getMaterial(index).getName();
		qs = QString::fromStdString(s);
		this->capMaterialValue->setCurrentText(qs);
	}
	this->collarXValue->setValue(stem->getSwelling().x);
	this->collarYValue->setValue(stem->getSwelling().y);

	enable(true);
	blockSignals(false);
}

void StemEditor::blockSignals(bool block)
{
	this->stemGroup->blockSignals(block);
	this->resolutionValue->blockSignals(block);
	this->divisionValue->blockSignals(block);
	this->radiusValue->blockSignals(block);
	this->minRadiusValue->blockSignals(block);
	this->radiusCurveValue->blockSignals(block);
	this->degreeValue->blockSignals(block);
	this->stemMaterialValue->blockSignals(block);
	this->capMaterialValue->blockSignals(block);
	this->collarXValue->blockSignals(block);
	this->collarYValue->blockSignals(block);
	this->customValue->blockSignals(block);
}

void StemEditor::enable(bool enable)
{
	if (!enable) {
		indicateSimilarities(this->radiusLabel);
		indicateSimilarities(this->minRadiusLabel);
		indicateSimilarities(this->radiusCurveLabel);
		indicateSimilarities(this->resolutionLabel);
		indicateSimilarities(this->divisionLabel);
		indicateSimilarities(this->degreeLabel);
		indicateSimilarities(this->stemMaterialLabel);
		indicateSimilarities(this->capMaterialLabel);
		indicateSimilarities(this->collarXLabel);
		indicateSimilarities(this->collarYLabel);
		indicateSimilarities(this->customLabel);
	}
	this->radiusValue->setEnabled(enable);
	this->minRadiusValue->setEnabled(enable);
	this->radiusCurveValue->setEnabled(enable);
	this->resolutionValue->setEnabled(enable);
	this->divisionValue->setEnabled(enable);
	this->degreeValue->setEnabled(enable);
	this->stemMaterialValue->setEnabled(enable);
	this->capMaterialValue->setEnabled(enable);
	this->collarXValue->setEnabled(enable);
	this->collarYValue->setEnabled(enable);
	this->customValue->setEnabled(enable);
}

bool StemEditor::addCurve(pg::Curve curve)
{
	QString name = QString::fromStdString(curve.getName());
	if (this->radiusCurveValue->findText(name) < 0) {
		this->radiusCurveValue->blockSignals(true);
		this->radiusCurveValue->addItem(name);
		this->radiusCurveValue->blockSignals(false);
		return true;
	}
	return false;
}

void StemEditor::updateCurve(pg::Curve curve, unsigned index)
{
	QString name = QString::fromStdString(curve.getName());
	this->radiusCurveValue->setItemText(index, name);
}

void StemEditor::removeCurve(unsigned index)
{
	this->radiusCurveValue->blockSignals(true);
	unsigned curveIndex = this->radiusCurveValue->currentIndex();
	if (index == curveIndex)
		this->radiusCurveValue->setCurrentIndex(0);
	this->radiusCurveValue->removeItem(index);
	this->radiusCurveValue->blockSignals(false);
}

bool StemEditor::addMaterial(ShaderParams params)
{
	QString name = QString::fromStdString(params.getName());
	if (this->stemMaterialValue->findText(name) < 0) {
		blockSignals(true);
		this->stemMaterialValue->addItem(name);
		this->capMaterialValue->addItem(name);
		blockSignals(false);
		return true;
	}
	return false;
}

void StemEditor::updateMaterials()
{
	unsigned size = this->shared->getMaterialCount();
	for (unsigned i = 0; i < size; i++) {
		ShaderParams params = this->shared->getMaterial(i);
		QString name = QString::fromStdString(params.getName());
		this->stemMaterialValue->setItemText(i, name);
		this->capMaterialValue->setItemText(i, name);
	}
}

void StemEditor::removeMaterial(unsigned index)
{
	/* Blocking signals is important because otherwise material indices
	will be decremented twice for selected stems. */
	blockSignals(true);
	unsigned stemIndex = this->stemMaterialValue->currentIndex();
	unsigned capIndex = this->capMaterialValue->currentIndex();
	if (index == stemIndex)
		this->stemMaterialValue->setCurrentIndex(0);
	if (index == capIndex)
		this->capMaterialValue->setCurrentIndex(0);
	this->stemMaterialValue->removeItem(index);
	this->capMaterialValue->removeItem(index);
	blockSignals(false);
}

void StemEditor::changeCustom(int custom)
{
	beginChanging();
	indicateSimilarities(this->customLabel);
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances)
		instance.first->setCustom(custom);
	finishChanging();
}

void StemEditor::changePathDegree(int i)
{
	beginChanging();
	indicateSimilarities(this->degreeLabel);
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

void StemEditor::changeResolution(int i)
{
	beginChanging();
	indicateSimilarities(this->resolutionLabel);
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances)
		instance.first->setResolution(i);
	this->editor->change();
}

void StemEditor::changeDivisions(int i)
{
	beginChanging();
	indicateSimilarities(this->divisionLabel);
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances) {
		pg::Path path = instance.first->getPath();
		path.setResolution(i);
		instance.first->setPath(path);
	}
	this->editor->change();
}

void StemEditor::changeRadius(double d)
{
	beginChanging();
	indicateSimilarities(this->radiusLabel);
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances)
		instance.first->setMaxRadius(d);
	this->editor->change();
}

void StemEditor::changeMinRadius(double d)
{
	beginChanging();
	indicateSimilarities(this->minRadiusLabel);
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances)
		instance.first->setMinRadius(d);
	this->editor->change();
}

void StemEditor::changeRadiusCurve(int curve)
{
	beginChanging();
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances)
		instance.first->setRadiusCurve(curve);
	this->editor->change();
	finishChanging();
}

void StemEditor::changeStemMaterial()
{
	beginChanging();
	indicateSimilarities(this->stemMaterialLabel);
	unsigned index = this->stemMaterialValue->currentIndex();
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances)
		instance.first->setMaterial(Stem::Outer, index);
	this->editor->change();
	finishChanging();
}

void StemEditor::changeXCollar(double d)
{
	beginChanging();
	indicateSimilarities(this->collarXLabel);
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances) {
		Vec2 swelling = instance.first->getSwelling();
		swelling.x = d;
		instance.first->setSwelling(swelling);
	}
	this->editor->change();
}

void StemEditor::changeYCollar(double d)
{
	beginChanging();
	indicateSimilarities(this->collarYLabel);
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances) {
		Vec2 swelling = instance.first->getSwelling();
		swelling.y = d;
		instance.first->setSwelling(swelling);
	}
	this->editor->change();
}

void StemEditor::changeCapMaterial()
{
	beginChanging();
	indicateSimilarities(this->capMaterialLabel);
	unsigned index = this->capMaterialValue->currentIndex();
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances)
		instance.first->setMaterial(Stem::Inner, index);
	this->editor->change();
	finishChanging();
}

void StemEditor::beginChanging()
{
	if (!this->saveStem) {
		this->saveStem = new SaveStem(this->editor->getSelection());
		this->saveStem->execute();
	}
}

void StemEditor::finishChanging()
{
	if (this->saveStem && !this->saveStem->isSameAsCurrent()) {
		this->saveStem->setNewSelection();
		this->editor->add(this->saveStem);
		/* The history will delete the command. */
		this->saveStem = nullptr;
	}
}

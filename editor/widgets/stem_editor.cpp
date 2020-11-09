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
#include "definitions.h"
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
	this->stemGroup = new QGroupBox("Stem", this);
	this->stemGroup->setSizePolicy(
		QSizePolicy::Expanding, QSizePolicy::Minimum);

	QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
	layout->setMargin(0);
	layout->setSpacing(0);
	layout->addWidget(this->stemGroup);

	QFormLayout *form = new QFormLayout(this->stemGroup);
	form->setSpacing(UI_FORM_SPACING);
	form->setMargin(UI_FORM_MARGIN);

	this->radiusLabel = new QLabel("Radius");
	this->radiusValue = new QDoubleSpinBox;
	this->radiusValue->setSingleStep(0.001);
	this->radiusValue->setDecimals(3);
	form->addRow(this->radiusLabel, this->radiusValue);

	this->minRadiusLabel = new QLabel("Min Radius");
	this->minRadiusValue = new QDoubleSpinBox;
	this->minRadiusValue->setSingleStep(0.001);
	this->minRadiusValue->setDecimals(3);
	form->addRow(this->minRadiusLabel, this->minRadiusValue);

	this->radiusCurveLabel = new QLabel("Radius Curve");
	this->radiusCurveValue = new QComboBox;
	form->addRow(this->radiusCurveLabel, this->radiusCurveValue);

	this->pDivisionLabel = new QLabel("Path Divisions");
	this->pDivisionValue = new QSpinBox;
	this->pDivisionValue->setMinimum(0);
	form->addRow(this->pDivisionLabel, this->pDivisionValue);

	this->sDivisionLabel = new QLabel("Section Divisions");
	this->sDivisionValue = new QSpinBox;
	this->sDivisionValue->setMinimum(3);
	form->addRow(this->sDivisionLabel, this->sDivisionValue);

	this->cDivisionLabel = new QLabel("Collar Divisions");
	this->cDivisionValue = new QSpinBox;
	this->cDivisionValue->setMinimum(0);
	form->addRow(this->cDivisionLabel, this->cDivisionValue);

	this->degreeLabel = new QLabel("Degree");
	this->degreeValue = new QComboBox;
	this->degreeValue->addItem(QString("Linear"));
	this->degreeValue->addItem(QString("Cubic"));
	form->addRow(this->degreeLabel, this->degreeValue);

	this->stemMaterialLabel = new QLabel("Material");
	this->stemMaterialValue = new QComboBox;
	form->addRow(this->stemMaterialLabel, this->stemMaterialValue);

	this->capMaterialLabel = new QLabel("Cap Material");
	this->capMaterialValue = new QComboBox;
	form->addRow(this->capMaterialLabel, this->capMaterialValue);

	this->collarXLabel = new QLabel("Collar.X");
	this->collarXValue = new QDoubleSpinBox;
	this->collarXValue->setSingleStep(0.1);
	this->collarXValue->setDecimals(3);
	form->addRow(this->collarXLabel, this->collarXValue);

	this->collarYLabel = new QLabel("Collar.Y");
	this->collarYValue = new QDoubleSpinBox;
	this->collarYValue->setSingleStep(0.1);
	this->collarYValue->setDecimals(3);
	form->addRow(this->collarYLabel, this->collarYValue);

	this->customLabel = new QLabel("Manual");
	this->customValue = new QCheckBox;
	form->addRow(this->customLabel, this->customValue);

	setValueWidths(form);

	connect(this->customValue, SIGNAL(stateChanged(int)),
		this, SLOT(changeCustom(int)));
	connect(this->degreeValue, SIGNAL(currentIndexChanged(int)),
		this, SLOT(changePathDegree(int)));
	connect(this->sDivisionValue, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
	connect(this->sDivisionValue, SIGNAL(valueChanged(int)),
		this, SLOT(changeSectionDivisions(int)));
	connect(this->pDivisionValue, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
	connect(this->pDivisionValue, SIGNAL(valueChanged(int)),
		this, SLOT(changePathDivisions(int)));
	connect(this->cDivisionValue, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
	connect(this->cDivisionValue, SIGNAL(valueChanged(int)),
		this, SLOT(changeCollarDivisions(int)));
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
		Stem *s1 = prev(it)->first;
		Stem *s2 = it->first;

		if (s1->isCustom() != s2->isCustom())
			indicateDifferences(this->customLabel);
		if (s1->getSectionDivisions() != s2->getSectionDivisions())
			indicateDifferences(this->sDivisionLabel);
		if (s1->getCollarDivisions() != s2->getCollarDivisions())
			indicateDifferences(this->cDivisionLabel);
		if (s1->getMaxRadius() != s2->getMaxRadius())
			indicateDifferences(this->radiusLabel);
		if (s1->getMinRadius() != s2->getMinRadius())
			indicateDifferences(this->minRadiusLabel);
		if (s1->getRadiusCurve() != s2->getRadiusCurve())
			indicateDifferences(this->radiusCurveLabel);

		pg::Path path1 = s1->getPath();
		pg::Path path2 = s2->getPath();
		if (path1.getDivisions() != path2.getDivisions())
			indicateDifferences(this->pDivisionLabel);

		Spline spline1 = path1.getSpline();
		Spline spline2 = path2.getSpline();
		if (spline1.getDegree() != spline2.getDegree())
			indicateDifferences(this->degreeLabel);

		unsigned material1 = s1->getMaterial(Stem::Outer);
		unsigned material2 = s2->getMaterial(Stem::Outer);
		if (material1 != material2)
			indicateDifferences(this->stemMaterialLabel);
		material1 = s1->getMaterial(Stem::Inner);
		material2 = s2->getMaterial(Stem::Inner);
		if (material1 != material2)
			indicateDifferences(this->capMaterialLabel);

		if (s1->getSwelling().x != s2->getSwelling().x)
			indicateDifferences(this->collarXLabel);
		if (s1->getSwelling().y != s2->getSwelling().y)
			indicateDifferences(this->collarYLabel);
	}

	this->customValue->setCheckState(
		stem->isCustom() ? Qt::Checked : Qt::Unchecked);
	this->sDivisionValue->setValue(stem->getSectionDivisions());
	this->cDivisionValue->setValue(stem->getCollarDivisions());
	this->pDivisionValue->setValue(stem->getPath().getDivisions());
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
	this->sDivisionValue->blockSignals(block);
	this->cDivisionValue->blockSignals(block);
	this->pDivisionValue->blockSignals(block);
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
		indicateSimilarities(this->sDivisionLabel);
		indicateSimilarities(this->cDivisionLabel);
		indicateSimilarities(this->pDivisionLabel);
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
	this->sDivisionValue->setEnabled(enable);
	this->cDivisionValue->setEnabled(enable);
	this->pDivisionValue->setEnabled(enable);
	this->degreeValue->setEnabled(enable);
	this->stemMaterialValue->setEnabled(enable);
	this->capMaterialValue->setEnabled(enable);
	this->collarXValue->setEnabled(enable);
	this->collarYValue->setEnabled(enable);
	this->customValue->setEnabled(enable);
}

void StemEditor::clearOptions()
{
	blockSignals(true);
	this->stemMaterialValue->clear();
	this->capMaterialValue->clear();
	this->radiusCurveValue->clear();
	blockSignals(false);
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
	beginChanging(this->customLabel);
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances)
		instance.first->setCustom(custom);
	finishChanging();
}

void StemEditor::changePathDegree(int index)
{
	beginChanging(this->degreeLabel);
	int degree = index == 1 ? 3 : 1;
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

void StemEditor::changeSectionDivisions(int divisions)
{
	beginChanging(this->sDivisionLabel);
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances)
		instance.first->setSectionDivisions(divisions);
	this->editor->change();
}

void StemEditor::changeCollarDivisions(int divisions)
{
	beginChanging(this->cDivisionLabel);
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances)
		instance.first->setCollarDivisions(divisions);
	this->editor->change();
}

void StemEditor::changePathDivisions(int divisions)
{
	beginChanging(this->pDivisionLabel);
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances) {
		pg::Path path = instance.first->getPath();
		path.setDivisions(divisions);
		instance.first->setPath(path);
	}
	this->editor->change();
}

void StemEditor::changeRadius(double radius)
{
	beginChanging(this->radiusLabel);
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances)
		instance.first->setMaxRadius(radius);
	this->editor->change();
}

void StemEditor::changeMinRadius(double minRadius)
{
	beginChanging(this->minRadiusLabel);
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances)
		instance.first->setMinRadius(minRadius);
	this->editor->change();
}

void StemEditor::changeRadiusCurve(int curve)
{
	beginChanging(this->radiusCurveLabel);
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances)
		instance.first->setRadiusCurve(curve);
	this->editor->change();
	finishChanging();
}

void StemEditor::changeStemMaterial()
{
	beginChanging(this->stemMaterialLabel);
	unsigned index = this->stemMaterialValue->currentIndex();
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances)
		instance.first->setMaterial(Stem::Outer, index);
	this->editor->change();
	finishChanging();
}

void StemEditor::changeXCollar(double xcollar)
{
	beginChanging(this->collarXLabel);
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances) {
		Vec2 swelling = instance.first->getSwelling();
		swelling.x = xcollar;
		instance.first->setSwelling(swelling);
	}
	this->editor->change();
}

void StemEditor::changeYCollar(double ycollar)
{
	beginChanging(this->collarYLabel);
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances) {
		Vec2 swelling = instance.first->getSwelling();
		swelling.y = ycollar;
		instance.first->setSwelling(swelling);
	}
	this->editor->change();
}

void StemEditor::changeCapMaterial()
{
	beginChanging(this->capMaterialLabel);
	unsigned index = this->capMaterialValue->currentIndex();
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances)
		instance.first->setMaterial(Stem::Inner, index);
	this->editor->change();
	finishChanging();
}

void StemEditor::beginChanging(QLabel *label)
{
	indicateSimilarities(label);
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

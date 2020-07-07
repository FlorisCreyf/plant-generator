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

#include "generator_editor.h"
#include "definitions.h"
#include <limits>

using pg::Derivation;
using pg::Stem;

GeneratorEditor::GeneratorEditor(Editor *editor, QWidget *parent) :
	Form(parent), editor(editor), generate(nullptr)
{
	createInterface();
	enable(false);
	connect(this->editor, SIGNAL(selectionChanged()),
		this, SLOT(setFields()));
}

QSize GeneratorEditor::sizeHint() const
{
	return QSize(UI_WIDGET_WIDTH, UI_WIDGET_HEIGHT);
}

void GeneratorEditor::createInterface()
{
	this->group = new QGroupBox(tr("Stem Generator"), this);
	this->group->setSizePolicy(
		QSizePolicy::Expanding, QSizePolicy::Minimum);

	QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
	layout->setMargin(0);
	layout->setSpacing(0);
	layout->addWidget(this->group);

	QFormLayout *form = new QFormLayout(this->group);
	form->setSizeConstraint(QLayout::SetMinimumSize);
	form->setSpacing(0);
	form->setMargin(0);
	form->setSpacing(UI_FORM_SPACING);
	form->setMargin(UI_FORM_MARGIN);

	this->seedLabel = new QLabel(tr("Seed"));
	this->seedValue = new QSpinBox(this);
	this->seedValue->setRange(
		std::numeric_limits<int>::min(),
		std::numeric_limits<int>::max());
	this->seedValue->setSingleStep(1);
	form->addRow(this->seedLabel, this->seedValue);

	this->stemDensityLabel = new QLabel(tr("Stem Density"));
	this->stemDensityValue = new QDoubleSpinBox(this);
	this->stemDensityValue->setSingleStep(0.1);
	form->addRow(this->stemDensityLabel, this->stemDensityValue);

	this->stemStartLabel = new QLabel(tr("Stem Start"));
	this->stemStartValue = new QDoubleSpinBox(this);
	this->stemStartValue->setSingleStep(0.1);
	form->addRow(this->stemStartLabel, this->stemStartValue);

	this->leafDensityLabel = new QLabel(tr("Leaf Density"));
	this->leafDensityValue = new QDoubleSpinBox();
	this->leafDensityValue->setSingleStep(0.1);
	form->addRow(this->leafDensityLabel, this->leafDensityValue);

	this->leafStartLabel = new QLabel(tr("Leaf Start"));
	this->leafStartValue = new QDoubleSpinBox(this);
	this->leafStartValue->setSingleStep(0.1);
	form->addRow(this->leafStartLabel, this->leafStartValue);

	this->arrangementLabel = new QLabel(tr("Arrangement"));
	this->arrangementValue = new QComboBox(this);
	this->arrangementValue->addItem(tr("Alternate"));
	this->arrangementValue->addItem(tr("Opposite"));
	this->arrangementValue->addItem(tr("Whorled"));
	form->addRow(this->arrangementLabel, this->arrangementValue);

	this->radiusThresholdLabel = new QLabel(tr("Radius Threshold"));
	this->radiusThresholdValue = new QDoubleSpinBox(this);
	this->radiusThresholdValue->setSingleStep(0.001);
	this->radiusThresholdValue->setDecimals(3);
	form->addRow(this->radiusThresholdLabel, this->radiusThresholdValue);

	this->lengthFactorLabel = new QLabel(tr("Length Factor"));
	this->lengthFactorValue = new QDoubleSpinBox(this);
	this->lengthFactorValue->setSingleStep(1.0f);
	this->lengthFactorValue->setRange(0, std::numeric_limits<float>::max());
	form->addRow(this->lengthFactorLabel, this->lengthFactorValue);

	this->depthLabel = new QLabel(tr("Depth"));
	this->depthValue = new QSpinBox(this);
	this->depthValue->setRange(1, 10);
	form->addRow(this->depthLabel, this->depthValue);

	setValueWidths(form);

	connect(this->seedValue, SIGNAL(valueChanged(int)),
		this, SLOT(changeSeed(int)));
	connect(this->seedValue, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
	connect(this->arrangementValue, SIGNAL(currentIndexChanged(int)),
		this, SLOT(changeArrangement(int)));
	connect(this->depthValue, SIGNAL(valueChanged(int)),
		this, SLOT(changeDepth(int)));
	connect(this->depthValue, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
	connect(this->stemDensityValue, SIGNAL(valueChanged(double)),
		this, SLOT(changeStemDensity(double)));
	connect(this->stemDensityValue, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
	connect(this->stemStartValue, SIGNAL(valueChanged(double)),
		this, SLOT(changeStemStart(double)));
	connect(this->stemStartValue, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
	connect(this->leafDensityValue, SIGNAL(valueChanged(double)),
		this, SLOT(changeLeafDensity(double)));
	connect(this->leafDensityValue, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
	connect(this->leafStartValue, SIGNAL(valueChanged(double)),
		this, SLOT(changeLeafStart(double)));
	connect(this->leafStartValue, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
	connect(this->radiusThresholdValue, SIGNAL(valueChanged(double)),
		this, SLOT(changeRadiusThreshold(double)));
	connect(this->radiusThresholdValue, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
	connect(this->lengthFactorValue, SIGNAL(valueChanged(double)),
		this, SLOT(changeLengthFactor(double)));
	connect(this->lengthFactorValue, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
}

void GeneratorEditor::setFields()
{
	enable(false);
	auto instances = this->editor->getSelection()->getStemInstances();
	if (instances.empty())
		return;

	blockSignals(true);
	auto it = instances.begin();
	Derivation derivation = it->first->getDerivation();
	Derivation dvn1 = it->first->getDerivation();
	while (++it != instances.end()) {
		Derivation dvn2 = it->first->getDerivation();
		if (dvn1.seed != dvn2.seed)
			indicateDifferences(this->seedLabel);
		if (dvn1.stemDensity != dvn2.stemDensity)
			indicateDifferences(this->stemDensityLabel);
		if (dvn1.stemStart != dvn2.stemStart)
			indicateDifferences(this->stemStartLabel);
		if (dvn1.leafDensity != dvn2.leafDensity)
			indicateDifferences(this->leafDensityLabel);
		if (dvn1.leafStart != dvn2.leafStart)
			indicateDifferences(this->leafStartLabel);
		if (dvn1.radiusThreshold != dvn2.radiusThreshold)
			indicateDifferences(this->radiusThresholdLabel);
		if (dvn1.depth != dvn2.depth)
			indicateDifferences(this->depthLabel);
		if (dvn1.lengthFactor != dvn2.lengthFactor)
			indicateDifferences(this->lengthFactorLabel);
		if (dvn1.arrangement != dvn2.arrangement)
			indicateDifferences(this->arrangementLabel);
		dvn1 = dvn2;
	}
	this->seedValue->setValue(derivation.seed);
	this->stemDensityValue->setValue(derivation.stemDensity);
	this->stemStartValue->setValue(derivation.stemStart);
	this->leafDensityValue->setValue(derivation.leafDensity);
	this->leafStartValue->setValue(derivation.leafStart);
	this->radiusThresholdValue->setValue(derivation.radiusThreshold);
	this->depthValue->setValue(derivation.depth);
	this->lengthFactorValue->setValue(derivation.lengthFactor);
	this->arrangementValue->setCurrentIndex(derivation.arrangement);
	enable(true);
	blockSignals(false);
}

void GeneratorEditor::enable(bool enable)
{
	if (!enable) {
		indicateSimilarities(this->seedLabel);
		indicateSimilarities(this->stemDensityLabel);
		indicateSimilarities(this->stemStartLabel);
		indicateSimilarities(this->leafDensityLabel);
		indicateSimilarities(this->leafStartLabel);
		indicateSimilarities(this->radiusThresholdLabel);
		indicateSimilarities(this->depthLabel);
		indicateSimilarities(this->lengthFactorLabel);
		indicateSimilarities(this->arrangementLabel);
	}
	this->seedValue->setEnabled(enable);
	this->stemDensityValue->setEnabled(enable);
	this->stemStartValue->setEnabled(enable);
	this->leafDensityValue->setEnabled(enable);
	this->leafStartValue->setEnabled(enable);
	this->radiusThresholdValue->setEnabled(enable);
	this->depthValue->setEnabled(enable);
	this->lengthFactorValue->setEnabled(enable);
	this->arrangementValue->setEnabled(enable);
}

void GeneratorEditor::blockSignals(bool block)
{
	this->seedValue->blockSignals(block);
	this->stemDensityValue->blockSignals(block);
	this->stemStartValue->blockSignals(block);
	this->leafDensityValue->blockSignals(block);
	this->leafStartValue->blockSignals(block);
	this->radiusThresholdValue->blockSignals(block);
	this->depthValue->blockSignals(block);
	this->lengthFactorValue->blockSignals(block);
	this->arrangementValue->blockSignals(block);
}

void GeneratorEditor::changeSeed(int seed)
{
	beginChanging(this->seedLabel);
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances) {
		Derivation dvn = instance.first->getDerivation();
		dvn.seed = seed;
		instance.first->setDerivation(dvn);
	}
	change();
}

void GeneratorEditor::changeStemDensity(double density)
{
	beginChanging(this->stemDensityLabel);
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances) {
		Derivation dvn = instance.first->getDerivation();
		dvn.stemDensity = density;
		instance.first->setDerivation(dvn);
	}
	change();
}

void GeneratorEditor::changeLeafDensity(double density)
{
	beginChanging(this->leafDensityLabel);
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances) {
		Derivation dvn = instance.first->getDerivation();
		dvn.leafDensity = density;
		instance.first->setDerivation(dvn);
	}
	change();
}

void GeneratorEditor::changeStemStart(double start)
{
	beginChanging(this->stemStartLabel);
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances) {
		Derivation dvn = instance.first->getDerivation();
		dvn.stemStart = start;
		instance.first->setDerivation(dvn);
	}
	change();
}

void GeneratorEditor::changeLeafStart(double start)
{
	beginChanging(this->leafStartLabel);
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances) {
		Derivation dvn = instance.first->getDerivation();
		dvn.leafStart = start;
		instance.first->setDerivation(dvn);
	}
	change();
}

void GeneratorEditor::changeRadiusThreshold(double threshold)
{
	beginChanging(this->radiusThresholdLabel);
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances) {
		Derivation dvn = instance.first->getDerivation();
		dvn.radiusThreshold = threshold;
		instance.first->setDerivation(dvn);
	}
	change();
}

void GeneratorEditor::changeLengthFactor(double factor)
{
	beginChanging(this->lengthFactorLabel);
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances) {
		Derivation dvn = instance.first->getDerivation();
		dvn.lengthFactor = factor;
		instance.first->setDerivation(dvn);
	}
	change();
}

void GeneratorEditor::changeArrangement(int index)
{
	beginChanging(this->arrangementLabel);
	auto instances = this->editor->getSelection()->getStemInstances();
	Derivation::Arrangement arrangement;
	arrangement = static_cast<pg::Derivation::Arrangement>(index);
	for (auto &instance : instances) {
		Derivation dvn = instance.first->getDerivation();
		dvn.arrangement = arrangement;
		instance.first->setDerivation(dvn);
	}
	change();
	finishChanging();
}

void GeneratorEditor::changeDepth(int depth)
{
	beginChanging(this->depthLabel);
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto &instance : instances) {
		Derivation dvn = instance.first->getDerivation();
		dvn.depth = depth;
		instance.first->setDerivation(dvn);
	}
	change();
}

void GeneratorEditor::change()
{
	this->generate->execute();
	this->editor->change();
}

void GeneratorEditor::beginChanging(QLabel *label)
{
	indicateSimilarities(label);
	if (!this->generate)
		this->generate = new Generate(this->editor->getSelection());
}

void GeneratorEditor::finishChanging()
{
	if (this->generate) {
		this->editor->add(this->generate);
		/* The history will delete the command. */
		this->generate = nullptr;
	}
}

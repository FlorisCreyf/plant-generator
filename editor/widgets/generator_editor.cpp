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

GeneratorEditor::GeneratorEditor(Editor *editor, QWidget *parent) : Form(parent)
{
	this->editor = editor;
	this->generate = nullptr;
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
	QFormLayout *form = new QFormLayout(this);
	form->setSizeConstraint(QLayout::SetMinimumSize);
	form->setSpacing(0);
	form->setMargin(0);
	form->setSpacing(2);
	form->setMargin(5);

	this->seed = new QSpinBox(this);
	this->seed->setRange(
		std::numeric_limits<int>::min(),
		std::numeric_limits<int>::max());
	this->seed->setSingleStep(1);
	form->addRow(tr("Seed"), this->seed);
	this->stemDensity = new QDoubleSpinBox(this);
	this->stemDensity->setSingleStep(0.1);
	form->addRow(tr("Stem Density"), this->stemDensity);
	this->stemStart = new QDoubleSpinBox(this);
	this->stemStart->setSingleStep(0.1);
	form->addRow(tr("Stem Start"), this->stemStart);
	this->leafDensity = new QDoubleSpinBox(this);
	this->leafDensity->setSingleStep(0.1);
	form->addRow(tr("Leaf Density"), this->leafDensity);
	this->leafStart = new QDoubleSpinBox(this);
	this->leafStart->setSingleStep(0.1);
	form->addRow(tr("Leaf Start"), this->leafStart);
	this->arrangement = new QComboBox(this);
	this->arrangement->addItem(tr("Alternate"));
	this->arrangement->addItem(tr("Opposite"));
	this->arrangement->addItem(tr("Whorled"));
	form->addRow(tr("Arrangment"), arrangement);
	this->radiusThreshold = new QDoubleSpinBox(this);
	this->radiusThreshold->setSingleStep(0.001);
	this->radiusThreshold->setDecimals(3);
	form->addRow(tr("Radius Threshold"), this->radiusThreshold);
	this->lengthFactor = new QDoubleSpinBox(this);
	this->lengthFactor->setSingleStep(0.1);
	form->addRow(tr("Length Factor"), this->lengthFactor);
	this->depth = new QSpinBox(this);
	form->addRow(tr("Depth"), this->depth);

	setValueWidths(form);

	connect(this->arrangement, SIGNAL(currentIndexChanged(int)),
		this, SLOT(changeOption()));

	connectField(this->seed);
	connectField(this->depth);
	connectField(this->stemDensity);
	connectField(this->stemStart);
	connectField(this->leafDensity);
	connectField(this->leafStart);
	connectField(this->radiusThreshold);
	connectField(this->lengthFactor);
}

void GeneratorEditor::connectField(QSpinBox *field)
{
	connect(field, SIGNAL(valueChanged(int)),
		this, SLOT(change()));
	connect(field, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
}

void GeneratorEditor::connectField(QDoubleSpinBox *field)
{
	connect(field, SIGNAL(valueChanged(double)),
		this, SLOT(change()));
	connect(field, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
}

void GeneratorEditor::setFields()
{
	auto instances = this->editor->getSelection()->getStemInstances();
	if (instances.empty()) {
		enable(false);
		return;
	}

	blockSignals(true);
	pg::Stem *stem = instances.rbegin()->first;
	pg::Derivation derivation = stem->getDerivation();
	this->seed->setValue(derivation.seed);
	this->stemDensity->setValue(derivation.stemDensity);
	this->stemStart->setValue(derivation.stemStart);
	this->leafDensity->setValue(derivation.leafDensity);
	this->leafStart->setValue(derivation.leafStart);
	this->radiusThreshold->setValue(derivation.radiusThreshold);
	this->depth->setValue(derivation.depth);
	this->lengthFactor->setValue(derivation.lengthFactor);
	this->arrangement->setCurrentIndex(derivation.arrangement);
	enable(true);
	blockSignals(false);
}

void GeneratorEditor::enable(bool enable)
{
	this->seed->setEnabled(enable);
	this->stemDensity->setEnabled(enable);
	this->stemStart->setEnabled(enable);
	this->leafDensity->setEnabled(enable);
	this->leafStart->setEnabled(enable);
	this->radiusThreshold->setEnabled(enable);
	this->depth->setEnabled(enable);
	this->lengthFactor->setEnabled(enable);
	this->arrangement->setEnabled(enable);
}

void GeneratorEditor::blockSignals(bool block)
{
	this->seed->blockSignals(block);
	this->stemDensity->blockSignals(block);
	this->stemStart->blockSignals(block);
	this->leafDensity->blockSignals(block);
	this->leafStart->blockSignals(block);
	this->radiusThreshold->blockSignals(block);
	this->depth->blockSignals(block);
	this->lengthFactor->blockSignals(block);
	this->arrangement->blockSignals(block);
}

void GeneratorEditor::change()
{
	if (!this->generate)
		this->generate = new Generate(this->editor->getSelection());
	pg::PseudoGenerator gen(this->editor->getPlant());
	pg::Derivation dvn;
	dvn.leafDensity = this->leafDensity->value();
	dvn.leafStart = this->leafStart->value();
	dvn.stemDensity = this->stemDensity->value();
	dvn.stemStart = this->stemStart->value();
	dvn.radiusThreshold = this->radiusThreshold->value();
	dvn.seed = this->seed->value();
	dvn.depth = this->depth->value();
	dvn.lengthFactor = this->lengthFactor->value();
	int index = this->arrangement->currentIndex();
	dvn.arrangement = static_cast<pg::Derivation::Arrangement>(index);
	gen.setDerivation(dvn);
	this->generate->setGenerator(std::move(gen));
	this->generate->execute();
	this->editor->change();
}

void GeneratorEditor::finishChanging()
{
	if (this->generate) {
		this->editor->add(this->generate);
		/* The history will delete the command. */
		this->generate = nullptr;
	}
}

void GeneratorEditor::changeOption()
{
	change();
	finishChanging();
}

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

using pg::DerivationTree;
using pg::DerivationNode;
using pg::Derivation;
using pg::Stem;
using std::string;

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

	this->nodeValue = new QComboBox(this);
	form->addRow(tr("Node"), this->nodeValue);

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

	this->leafScaleValue[0] = new QDoubleSpinBox(this);
	this->leafScaleValue[1] = new QDoubleSpinBox(this);
	this->leafScaleValue[2] = new QDoubleSpinBox(this);
	this->leafScaleLabel[0] = new QLabel(tr("Leaf Scale.X"));
	this->leafScaleLabel[1] = new QLabel(tr("Leaf Scale.Y"));
	this->leafScaleLabel[2] = new QLabel(tr("Leaf Scale.Z"));
	form->addRow(this->leafScaleLabel[0], this->leafScaleValue[0]);
	form->addRow(this->leafScaleLabel[1], this->leafScaleValue[1]);
	form->addRow(this->leafScaleLabel[2], this->leafScaleValue[2]);

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

	this->childButton = new QPushButton(tr("Add Child Node"), this);
	form->addRow("", this->childButton);
	this->siblingButton = new QPushButton(tr("Add Sibling Node"), this);
	form->addRow("", this->siblingButton);
	this->removeButton = new QPushButton(tr("Remove Node"), this);
	form->addRow("", this->removeButton);

	setValueWidths(form);

	connect(this->nodeValue, SIGNAL(currentIndexChanged(int)),
		this, SLOT(select()));
	connect(this->childButton, SIGNAL(clicked()),
		this, SLOT(addChildNode()));
	connect(this->siblingButton, SIGNAL(clicked()),
		this, SLOT(addSiblingNode()));
	connect(this->removeButton, SIGNAL(clicked()),
		this, SLOT(removeNode()));
	connect(this->seedValue, SIGNAL(valueChanged(int)),
		this, SLOT(change()));
	connect(this->seedValue, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
	connect(this->arrangementValue, SIGNAL(currentIndexChanged(int)),
		this, SLOT(changeOnce()));
	connect(this->stemDensityValue, SIGNAL(valueChanged(double)),
		this, SLOT(change()));
	connect(this->stemDensityValue, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
	connect(this->stemStartValue, SIGNAL(valueChanged(double)),
		this, SLOT(change()));
	connect(this->stemStartValue, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
	connect(this->leafDensityValue, SIGNAL(valueChanged(double)),
		this, SLOT(change()));
	connect(this->leafDensityValue, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
	connect(this->leafStartValue, SIGNAL(valueChanged(double)),
		this, SLOT(change()));
	connect(this->leafStartValue, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
	connect(this->radiusThresholdValue, SIGNAL(valueChanged(double)),
		this, SLOT(change()));
	connect(this->radiusThresholdValue, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
	connect(this->lengthFactorValue, SIGNAL(valueChanged(double)),
		this, SLOT(change()));
	connect(this->lengthFactorValue, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
	for (int i = 0; i < 3; i++) {
		connect(this->leafScaleValue[i], SIGNAL(valueChanged(double)),
			this, SLOT(change()));
		connect(this->leafScaleValue[i], SIGNAL(editingFinished()),
			this, SLOT(finishChanging()));
	}
}

void GeneratorEditor::setFields()
{
	enable(false);
	auto instances = this->editor->getSelection()->getStemInstances();
	if (instances.empty())
		return;
	DerivationTree dvnTree = instances.begin()->first->getDerivation();
	setFields(dvnTree, "");
	enable(true);
}

void GeneratorEditor::setFields(const DerivationTree &dvnTree, string name)
{
	blockSignals(true);
	DerivationNode *dvnNode = dvnTree.get(name);
	Derivation derivation;
	this->nodeValue->clear();
	if (dvnNode) {
		std::vector<string> names = dvnTree.getNames();
		for (string name : names)
			this->nodeValue->addItem(QString::fromStdString(name));
		derivation = dvnNode->getData();
	} else
		this->nodeValue->addItem("1");

	if (!name.empty())
		this->nodeValue->setCurrentText(QString::fromStdString(name));

	this->seedValue->setValue(dvnTree.getSeed());
	this->stemDensityValue->setValue(derivation.stemDensity);
	this->stemStartValue->setValue(derivation.stemStart);
	this->leafDensityValue->setValue(derivation.leafDensity);
	this->leafStartValue->setValue(derivation.leafStart);
	this->leafScaleValue[0]->setValue(derivation.leafScale.x);
	this->leafScaleValue[1]->setValue(derivation.leafScale.y);
	this->leafScaleValue[2]->setValue(derivation.leafScale.z);
	this->radiusThresholdValue->setValue(derivation.radiusThreshold);
	this->lengthFactorValue->setValue(derivation.lengthFactor);
	this->arrangementValue->setCurrentIndex(derivation.arrangement);
	blockSignals(false);
}

void GeneratorEditor::enable(bool enable)
{
	this->seedValue->setEnabled(enable);
	this->stemDensityValue->setEnabled(enable);
	this->stemStartValue->setEnabled(enable);
	this->leafDensityValue->setEnabled(enable);
	this->leafStartValue->setEnabled(enable);
	this->radiusThresholdValue->setEnabled(enable);
	this->lengthFactorValue->setEnabled(enable);
	this->arrangementValue->setEnabled(enable);
	this->leafScaleValue[0]->setEnabled(enable);
	this->leafScaleValue[1]->setEnabled(enable);
	this->leafScaleValue[2]->setEnabled(enable);
	this->childButton->setEnabled(enable);
	this->siblingButton->setEnabled(enable);
	this->removeButton->setEnabled(enable);
	this->nodeValue->setEnabled(enable);
}

void GeneratorEditor::blockSignals(bool block)
{
	this->seedValue->blockSignals(block);
	this->stemDensityValue->blockSignals(block);
	this->stemStartValue->blockSignals(block);
	this->leafDensityValue->blockSignals(block);
	this->leafStartValue->blockSignals(block);
	this->radiusThresholdValue->blockSignals(block);
	this->lengthFactorValue->blockSignals(block);
	this->arrangementValue->blockSignals(block);
	this->leafScaleValue[0]->blockSignals(block);
	this->leafScaleValue[1]->blockSignals(block);
	this->leafScaleValue[2]->blockSignals(block);
	this->nodeValue->blockSignals(block);
}

void GeneratorEditor::change()
{
	auto instances = this->editor->getSelection()->getStemInstances();
	if (instances.empty())
		return;

	beginChanging();
	Stem *stem = instances.begin()->first;
	DerivationTree dvnTree = stem->getDerivation();
	dvnTree.setSeed(this->seedValue->value());

	DerivationNode *dvnNode;
	if (dvnTree.getRoot()) {
		std::string name = this->nodeValue->currentText().toStdString();
		dvnNode = dvnTree.get(name);
	} else
		dvnNode = dvnTree.createRoot();
	Derivation dvn = dvnNode->getData();

	dvn.stemDensity = this->stemDensityValue->value();
	dvn.leafDensity = this->leafDensityValue->value();
	dvn.stemStart = this->stemStartValue->value();
	dvn.leafStart = this->leafStartValue->value();
	dvn.lengthFactor = this->lengthFactorValue->value();
	dvn.radiusThreshold = this->radiusThresholdValue->value();
	int arrangement = this->arrangementValue->currentIndex();
	dvn.arrangement = static_cast<Derivation::Arrangement>(arrangement);
	dvn.leafScale.x = this->leafScaleValue[0]->value();
	dvn.leafScale.y = this->leafScaleValue[1]->value();
	dvn.leafScale.z = this->leafScaleValue[2]->value();
	dvnNode->setData(dvn);

	for (auto instance : instances)
		instance.first->setDerivation(dvnTree);

	this->generate->execute();
	this->editor->change();
}

void GeneratorEditor::changeOnce()
{
	change();
	finishChanging();
}

void GeneratorEditor::beginChanging()
{
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

void GeneratorEditor::select()
{
	auto instances = this->editor->getSelection()->getStemInstances();
	if (instances.empty())
		return;

	Stem *stem = instances.begin()->first;
	DerivationTree dvnTree = stem->getDerivation();
	setFields(dvnTree, this->nodeValue->currentText().toStdString());
}

void GeneratorEditor::addChildNode()
{
	auto instances = this->editor->getSelection()->getStemInstances();
	if (instances.empty())
		return;

	beginChanging();
	Stem *stem = instances.begin()->first;
	DerivationTree dvnTree = stem->getDerivation();
	string name = this->nodeValue->currentText().toStdString();
	dvnTree.addChild(name);
	setFields(dvnTree, name + ".1");
	for (auto it = instances.begin(); it != instances.end(); it++)
		it->first->setDerivation(dvnTree);

	changeOnce();
	emit derivationModified();
}

void GeneratorEditor::addSiblingNode()
{
	auto instances = this->editor->getSelection()->getStemInstances();
	if (instances.empty())
		return;

	beginChanging();
	Stem *stem = instances.begin()->first;
	DerivationTree dvnTree = stem->getDerivation();
	string name = this->nodeValue->currentText().toStdString();
	dvnTree.addSibling(name);

	int size;
	size_t index = name.rfind('.');
	if (index != string::npos) {
		size = stoi(name.substr(index+1, name.size()-index-1));
		name.erase(index+1, name.size()-index-1);
	} else {
		size = stoi(name);
		name.clear();
	}

	setFields(dvnTree, name + std::to_string(size+1));
	for (auto it = instances.begin(); it != instances.end(); it++)
		it->first->setDerivation(dvnTree);

	changeOnce();
	emit derivationModified();
}

void GeneratorEditor::removeNode()
{
	auto instances = this->editor->getSelection()->getStemInstances();
	if (instances.empty())
		return;

	beginChanging();
	Stem *stem = instances.begin()->first;
	DerivationTree dvnTree = stem->getDerivation();
	string name = this->nodeValue->currentText().toStdString();
	if (name == "1")
		return;

	dvnTree.remove(name);
	setFields(dvnTree, "");
	for (auto it = instances.begin(); it != instances.end(); it++)
		it->first->setDerivation(dvnTree);

	changeOnce();
	emit derivationModified();
}

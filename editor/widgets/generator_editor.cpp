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

const float pi = 3.14159265359f;

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
	QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
	layout->setMargin(0);
	layout->setSpacing(0);
	createNodeGroup(layout);
	createStemGroup(layout);
	createLeafGroup(layout);
}

void GeneratorEditor::createNodeGroup(QBoxLayout *layout)
{
	this->nodeGroup = new QGroupBox("Nodes", this);
	this->nodeGroup->setSizePolicy(
		QSizePolicy::Expanding, QSizePolicy::Minimum);
	QFormLayout *form = createForm(this->nodeGroup);

	this->seedValue = new QSpinBox();
	this->seedValue->setRange(
		std::numeric_limits<int>::min(),
		std::numeric_limits<int>::max());
	form->addRow("Seed", this->seedValue);
	this->nodeValue = new QComboBox(this);
	form->addRow("Node", this->nodeValue);
	this->childButton = new QPushButton(tr("Add Child Node"), this);
	form->addRow("", this->childButton);
	this->siblingButton = new QPushButton(tr("Add Sibling Node"), this);
	form->addRow("", this->siblingButton);
	this->removeButton = new QPushButton(tr("Remove Node"), this);
	form->addRow("", this->removeButton);

	setValueWidths(form);
	layout->addWidget(this->nodeGroup);

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
}

void GeneratorEditor::createStemGroup(QBoxLayout *layout)
{
	this->stemGroup = new QGroupBox("Stems", this);
	this->stemGroup->setSizePolicy(
		QSizePolicy::Expanding, QSizePolicy::Minimum);
	QFormLayout *form = createForm(this->stemGroup);

	for (int i = 0; i < dssize; i++) {
		this->dsv[i] = new QDoubleSpinBox(this);
		this->dsv[i]->setSingleStep(0.1);
	}

	this->dsl[StemDensity] = new QLabel("Stems/Unit");
	this->dsl[StemStart] = new QLabel("Start");
	this->dsl[RadiusThreshold] = new QLabel(tr("Radius Threshold"));
	this->dsv[RadiusThreshold]->setSingleStep(0.001);
	this->dsv[RadiusThreshold]->setDecimals(3);
	this->dsl[LengthFactor] = new QLabel(tr("Length Factor"));
	this->dsv[LengthFactor]->setSingleStep(1.0f);
	this->dsv[LengthFactor]->setRange(0, std::numeric_limits<float>::max());

	for (int i = 0; i < dssize; i++)
		form->addRow(this->dsl[i], this->dsv[i]);

	setValueWidths(form);
	layout->addWidget(this->stemGroup);

	for (int i = 0; i < dssize; i++) {
		connect(this->dsv[i], SIGNAL(valueChanged(double)),
			this, SLOT(change()));
		connect(this->dsv[i], SIGNAL(editingFinished()),
			this, SLOT(finishChanging()));
	}
}

void GeneratorEditor::createLeafGroup(QBoxLayout *layout)
{
	this->leafGroup = new QGroupBox("Leaves", this);
	this->leafGroup->setSizePolicy(
		QSizePolicy::Expanding, QSizePolicy::Minimum);
	QFormLayout *form = createForm(this->leafGroup);

	for (int i = 0; i < dlsize; i++) {
		this->dlv[i] = new QDoubleSpinBox(this);
		this->dlv[i]->setSingleStep(0.1);
	}
	for (int i = 0; i < ilsize; i++) {
		this->ilv[i] = new QSpinBox(this);
		this->ilv[i]->setSingleStep(1);
	}

	this->dll[LeafDensity] = new QLabel("Nodes/Unit");
	this->dll[LeafDistance] = new QLabel("Distance");
	this->dll[LeafScaleX] = new QLabel("Scale.X");
	this->dll[LeafScaleY] = new QLabel("Scale.Y");
	this->dll[LeafScaleZ] = new QLabel("Scale.Z");
	this->dll[LeafRotation] = new QLabel("Rotation");
	this->dlv[LeafRotation]->setRange(
		std::numeric_limits<float>::min(),
		std::numeric_limits<float>::max());
	this->dll[MinUp] = new QLabel("Min Up");
	this->dll[MaxUp] = new QLabel("Max Up");
	this->dll[MinDirection] = new QLabel("Min Direction");
	this->dll[MaxDirection] = new QLabel("Max Direction");
	this->ill[LeavesPerNode] = new QLabel("Leaves/Node");

	for (int i = 0; i < dlsize; i++)
		form->addRow(this->dll[i], this->dlv[i]);
	for (int i = 0; i < ilsize; i++)
		form->addRow(this->ill[i], this->ilv[i]);

	setValueWidths(form);
	layout->addWidget(this->leafGroup);

	for (int i = 0; i < dlsize; i++) {
		connect(this->dlv[i], SIGNAL(valueChanged(double)),
			this, SLOT(change()));
		connect(this->dlv[i], SIGNAL(editingFinished()),
			this, SLOT(finishChanging()));
	}
	for (int i = 0; i < ilsize; i++) {
		connect(this->ilv[i], SIGNAL(valueChanged(int)),
			this, SLOT(change()));
		connect(this->ilv[i], SIGNAL(editingFinished()),
			this, SLOT(finishChanging()));
	}
}

QFormLayout *GeneratorEditor::createForm(QGroupBox *group)
{
	QFormLayout *form = new QFormLayout(group);
	form->setSizeConstraint(QLayout::SetMinimumSize);
	form->setSpacing(0);
	form->setMargin(0);
	form->setSpacing(UI_FORM_SPACING);
	form->setMargin(UI_FORM_MARGIN);
	return form;
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

	Derivation dvn;
	DerivationNode *dvnNode = dvnTree.get(name);
	this->nodeValue->clear();
	if (dvnNode) {
		std::vector<string> names = dvnTree.getNames();
		for (string name : names)
			this->nodeValue->addItem(QString::fromStdString(name));
		dvn = dvnNode->getData();
	} else
		this->nodeValue->addItem("1");

	if (!name.empty())
		this->nodeValue->setCurrentText(QString::fromStdString(name));

	this->seedValue->setValue(dvnTree.getSeed());
	this->dsv[StemDensity]->setValue(dvn.stemDensity);
	this->dsv[StemStart]->setValue(dvn.stemStart);
	this->dsv[RadiusThreshold]->setValue(dvn.radiusThreshold);
	this->dsv[LengthFactor]->setValue(dvn.lengthFactor);

	this->dlv[LeafDensity]->setValue(dvn.leafDensity);
	this->dlv[LeafDistance]->setValue(dvn.leafDistance);
	this->dlv[LeafScaleX]->setValue(dvn.leafScale.x);
	this->dlv[LeafScaleY]->setValue(dvn.leafScale.y);
	this->dlv[LeafScaleZ]->setValue(dvn.leafScale.z);
	this->dlv[LeafRotation]->setValue(dvn.leafRotation/pi*180.0f);
	this->dlv[MinUp]->setValue(dvn.minUp);
	this->dlv[MaxUp]->setValue(dvn.maxUp);
	this->dlv[MinDirection]->setValue(dvn.minDirection);
	this->dlv[MaxDirection]->setValue(dvn.maxDirection);
	this->ilv[LeavesPerNode]->setValue(dvn.leavesPerNode);

	blockSignals(false);
}

void GeneratorEditor::enable(bool enable)
{
	this->seedValue->setEnabled(enable);
	this->nodeValue->setEnabled(enable);
	this->childButton->setEnabled(enable);
	this->siblingButton->setEnabled(enable);
	this->removeButton->setEnabled(enable);
	for (int i = 0; i < dssize; i++)
		this->dsv[i]->setEnabled(enable);
	for (int i = 0; i < dlsize; i++)
		this->dlv[i]->setEnabled(enable);
	for (int i = 0; i < ilsize; i++)
		this->ilv[i]->setEnabled(enable);
}

void GeneratorEditor::blockSignals(bool block)
{
	this->seedValue->blockSignals(block);
	this->nodeValue->blockSignals(block);
	for (int i = 0; i < dssize; i++)
		this->dsv[i]->blockSignals(block);
	for (int i = 0; i < dlsize; i++)
		this->dlv[i]->blockSignals(block);
	for (int i = 0; i < ilsize; i++)
		this->ilv[i]->blockSignals(block);
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

	dvn.stemDensity = this->dsv[StemDensity]->value();
	dvn.stemStart = this->dsv[StemStart]->value();
	dvn.lengthFactor = this->dsv[LengthFactor]->value();
	dvn.radiusThreshold = this->dsv[RadiusThreshold]->value();

	dvn.leafDensity = this->dlv[LeafDensity]->value();
	dvn.leafDistance = this->dlv[LeafDistance]->value();
	dvn.leafScale.x = this->dlv[LeafScaleX]->value();
	dvn.leafScale.y = this->dlv[LeafScaleY]->value();
	dvn.leafScale.z = this->dlv[LeafScaleZ]->value();
	dvn.leafRotation = this->dlv[LeafRotation]->value()/180.0f*pi;
	dvn.minUp = this->dlv[MinUp]->value();
	dvn.maxUp = this->dlv[MaxUp]->value();
	dvn.minDirection = this->dlv[MinDirection]->value();
	dvn.maxDirection = this->dlv[MaxDirection]->value();
	dvn.leavesPerNode = this->ilv[LeavesPerNode]->value();

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

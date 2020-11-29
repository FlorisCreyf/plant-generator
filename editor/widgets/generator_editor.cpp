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
#include "form.h"
#include <limits>

using pg::ParameterTree;
using pg::ParameterNode;
using pg::ParameterRoot;
using pg::LeafData;
using pg::StemData;
using pg::Stem;
using std::string;

const float pi = 3.14159265359f;

GeneratorEditor::GeneratorEditor(Editor *editor, QWidget *parent) :
	QWidget(parent),
	editor(editor),
	generate(nullptr)
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
	createRootGroup(layout);
	createStemGroup(layout);
	createLeafGroup(layout);
}

void GeneratorEditor::createNodeGroup(QBoxLayout *layout)
{
	this->nodeGroup = new QGroupBox("Nodes", this);
	this->nodeGroup->setSizePolicy(
		QSizePolicy::Expanding, QSizePolicy::Minimum);
	QFormLayout *form = createForm(this->nodeGroup);

	this->nodeValue = new QComboBox(this);
	form->addRow(this->nodeValue);
	connect(this->nodeValue,
		QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, &GeneratorEditor::select);
	this->childButton = new QPushButton("Add Child Node", this);
	form->addRow(this->childButton);
	connect(this->childButton, &QPushButton::clicked,
		this, &GeneratorEditor::addChildNode);
	this->siblingButton = new QPushButton("Add Sibling Node", this);
	form->addRow(this->siblingButton);
	connect(this->siblingButton, &QPushButton::clicked,
		this, &GeneratorEditor::addSiblingNode);
	this->removeButton = new QPushButton("Remove Node", this);
	form->addRow(this->removeButton);
	connect(this->removeButton, &QPushButton::clicked,
		this, &GeneratorEditor::removeNode);

	setValueWidths(form);
	layout->addWidget(this->nodeGroup);
}

void GeneratorEditor::createRootGroup(QBoxLayout *layout)
{
	this->rootGroup = new QGroupBox("Root", this);
	this->rootGroup->setSizePolicy(
		QSizePolicy::Expanding, QSizePolicy::Minimum);
	QFormLayout *form = createForm(this->rootGroup);

	this->seedValue = new QSpinBox();
	this->seedValue->setRange(
		std::numeric_limits<int>::min(),
		std::numeric_limits<int>::max());
	form->addRow("Seed", this->seedValue);
	connect(this->seedValue,
		QOverload<int>::of(&QSpinBox::valueChanged),
		this, &GeneratorEditor::change);
	connect(this->seedValue, &QSpinBox::editingFinished,
		this, &GeneratorEditor::finishChanging);

	setValueWidths(form);
	layout->addWidget(this->rootGroup);
}

void GeneratorEditor::createStemGroup(QBoxLayout *layout)
{
	this->stemGroup = new QGroupBox("Stems", this);
	this->stemGroup->setSizePolicy(
		QSizePolicy::Expanding, QSizePolicy::Minimum);
	QFormLayout *form = createForm(this->stemGroup);

	for (int i = 0; i < DSSize; i++) {
		this->dsv[i] = new QDoubleSpinBox(this);
		this->dsv[i]->setSingleStep(0.1);
	}

	this->dsl[StemDensity] = new QLabel("Stems/Unit");
	this->dsl[StemStart] = new QLabel("Start");
	this->dsl[StemScale] = new QLabel("Scale");
	this->dsv[StemScale]->setSingleStep(0.001);
	this->dsv[StemScale]->setDecimals(3);
	this->dsl[StemAngleVariation] = new QLabel("Angle Variation");
	this->dsl[RadiusThreshold] = new QLabel("Radius Threshold");
	this->dsv[RadiusThreshold]->setSingleStep(0.001);
	this->dsv[RadiusThreshold]->setDecimals(3);
	this->dsl[StemLength] = new QLabel("Length");
	this->dsv[StemLength]->setSingleStep(1.0f);
	this->dsv[StemLength]->setRange(0, std::numeric_limits<float>::max());

	for (int i = 0; i < DSSize; i++)
		form->addRow(this->dsl[i], this->dsv[i]);

	setValueWidths(form);
	layout->addWidget(this->stemGroup);

	for (int i = 0; i < DSSize; i++) {
		connect(this->dsv[i],
			QOverload<double>::of(&QDoubleSpinBox::valueChanged),
			this, &GeneratorEditor::change);
		connect(this->dsv[i], &QDoubleSpinBox::editingFinished,
			this, &GeneratorEditor::finishChanging);
	}
}

void GeneratorEditor::createLeafGroup(QBoxLayout *layout)
{
	this->leafGroup = new QGroupBox("Leaves", this);
	this->leafGroup->setSizePolicy(
		QSizePolicy::Expanding, QSizePolicy::Minimum);
	QFormLayout *form = createForm(this->leafGroup);

	for (int i = 0; i < DLSize; i++) {
		this->dlv[i] = new QDoubleSpinBox(this);
		this->dlv[i]->setSingleStep(0.1);
	}
	for (int i = 0; i < ILSize; i++) {
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

	for (int i = 0; i < DLSize; i++)
		form->addRow(this->dll[i], this->dlv[i]);
	for (int i = 0; i < ILSize; i++)
		form->addRow(this->ill[i], this->ilv[i]);

	setValueWidths(form);
	layout->addWidget(this->leafGroup);

	for (int i = 0; i < DLSize; i++) {
		connect(this->dlv[i],
			QOverload<double>::of(&QDoubleSpinBox::valueChanged),
			this, &GeneratorEditor::change);
		connect(this->dlv[i], &QDoubleSpinBox::editingFinished,
			this, &GeneratorEditor::finishChanging);
	}
	for (int i = 0; i < ILSize; i++) {
		connect(this->ilv[i],
			QOverload<int>::of(&QSpinBox::valueChanged),
			this, &GeneratorEditor::change);
		connect(this->ilv[i], &QSpinBox::editingFinished,
			this, &GeneratorEditor::finishChanging);
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
	if (!instances.empty()) {
		ParameterTree tree;
		tree = instances.begin()->first->getParameterTree();
		ParameterRoot *root = tree.getRoot();
		if (root && root->getNode())
			setFields(tree, "1");
		else
			setFields(tree, "");
	}
}

void GeneratorEditor::setFields(const ParameterTree &tree, string name)
{
	blockSignals(true);

	StemData data;
	ParameterNode *node = name == "" ? nullptr : tree.get(name);

	this->nodeValue->clear();
	this->nodeValue->addItem("");
	if (tree.getRoot()) {
		std::vector<string> names = tree.getNames();
		for (string name : names)
			this->nodeValue->addItem(QString::fromStdString(name));
		this->seedValue->setValue(tree.getRoot()->getSeed());
	} else
		this->seedValue->setValue(0);

	if (node) {
		this->nodeValue->setCurrentText(QString::fromStdString(name));
		setStemData(node->getData());
	}

	enable(true);
	blockSignals(false);
}

void GeneratorEditor::setStemData(pg::StemData data)
{
	this->dsv[StemDensity]->setValue(data.density);
	this->dsv[StemStart]->setValue(data.start);
	this->dsv[StemScale]->setValue(data.scale);
	this->dsv[StemAngleVariation]->setValue(data.angleVariation);
	this->dsv[RadiusThreshold]->setValue(data.radiusThreshold);
	this->dsv[StemLength]->setValue(data.length);
	setLeafData(data.leaf);
}

void GeneratorEditor::setLeafData(pg::LeafData data)
{
	this->dlv[LeafDensity]->setValue(data.density);
	this->dlv[LeafDistance]->setValue(data.distance);
	this->dlv[LeafScaleX]->setValue(data.scale.x);
	this->dlv[LeafScaleY]->setValue(data.scale.y);
	this->dlv[LeafScaleZ]->setValue(data.scale.z);
	this->dlv[LeafRotation]->setValue(data.rotation/pi*180.0f);
	this->dlv[MinUp]->setValue(data.minUp);
	this->dlv[MaxUp]->setValue(data.maxUp);
	this->dlv[MinDirection]->setValue(data.minDirection);
	this->dlv[MaxDirection]->setValue(data.maxDirection);
	this->ilv[LeavesPerNode]->setValue(data.leavesPerNode);
}

void GeneratorEditor::enable(bool enable)
{
	this->seedValue->setEnabled(enable);
	this->nodeValue->setEnabled(enable);
	this->childButton->setEnabled(enable);
	this->siblingButton->setEnabled(enable);
	this->removeButton->setEnabled(enable);
	if (this->nodeValue->currentIndex() == 0)
		enable = false;
	for (int i = 0; i < DSSize; i++)
		this->dsv[i]->setEnabled(enable);
	for (int i = 0; i < DLSize; i++)
		this->dlv[i]->setEnabled(enable);
	for (int i = 0; i < ILSize; i++)
		this->ilv[i]->setEnabled(enable);
}

void GeneratorEditor::blockSignals(bool block)
{
	this->seedValue->blockSignals(block);
	this->nodeValue->blockSignals(block);
	for (int i = 0; i < DSSize; i++)
		this->dsv[i]->blockSignals(block);
	for (int i = 0; i < DLSize; i++)
		this->dlv[i]->blockSignals(block);
	for (int i = 0; i < ILSize; i++)
		this->ilv[i]->blockSignals(block);
}

void GeneratorEditor::change()
{
	auto instances = this->editor->getSelection()->getStemInstances();
	if (instances.empty())
		return;

	beginChanging();
	Stem *stem = instances.begin()->first;
	ParameterTree tree = stem->getParameterTree();
	if (!tree.getRoot())
		tree.createRoot();

	tree.getRoot()->setSeed(this->seedValue->value());
	if (this->nodeValue->currentIndex() > 0) {
		std::string name = this->nodeValue->currentText().toStdString();
		ParameterNode *node = tree.get(name);
		StemData data = getStemData(node->getData());
		node->setData(data);
	}

	for (auto instance : instances)
		instance.first->setParameterTree(tree);

	this->generate->execute();
	this->editor->change();
}

pg::StemData GeneratorEditor::getStemData(StemData data)
{
	data.density = this->dsv[StemDensity]->value();
	data.start = this->dsv[StemStart]->value();
	data.scale = this->dsv[StemScale]->value();
	data.angleVariation = this->dsv[StemAngleVariation]->value();
	data.length = this->dsv[StemLength]->value();
	data.radiusThreshold = this->dsv[RadiusThreshold]->value();
	data.leaf = getLeafData(data.leaf);
	return data;
}

pg::LeafData GeneratorEditor::getLeafData(LeafData data)
{
	data.density = this->dlv[LeafDensity]->value();
	data.distance = this->dlv[LeafDistance]->value();
	data.scale.x = this->dlv[LeafScaleX]->value();
	data.scale.y = this->dlv[LeafScaleY]->value();
	data.scale.z = this->dlv[LeafScaleZ]->value();
	data.rotation = this->dlv[LeafRotation]->value()/180.0f*pi;
	data.minUp = this->dlv[MinUp]->value();
	data.maxUp = this->dlv[MaxUp]->value();
	data.minDirection = this->dlv[MinDirection]->value();
	data.maxDirection = this->dlv[MaxDirection]->value();
	data.leavesPerNode = this->ilv[LeavesPerNode]->value();
	return data;
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
	ParameterTree tree = stem->getParameterTree();
	setFields(tree, this->nodeValue->currentText().toStdString());
}

void GeneratorEditor::addChildNode()
{
	auto instances = this->editor->getSelection()->getStemInstances();
	if (instances.empty())
		return;

	beginChanging();
	Stem *stem = instances.begin()->first;
	ParameterTree tree = stem->getParameterTree();

	if (!tree.getRoot())
		tree.createRoot();

	if (this->nodeValue->currentIndex() == 0) {
		tree.addChild("");
		setFields(tree, "1");
	} else {
		string name = this->nodeValue->currentText().toStdString();
		tree.addChild(name);
		setFields(tree, name + ".1");
	}

	for (auto it = instances.begin(); it != instances.end(); it++)
		it->first->setParameterTree(tree);

	changeOnce();
	emit parameterTreeModified();
}

void GeneratorEditor::addSiblingNode()
{
	auto instances = this->editor->getSelection()->getStemInstances();
	if (instances.empty() || this->nodeValue->currentIndex() <= 0)
		return;

	beginChanging();
	Stem *stem = instances.begin()->first;
	ParameterTree tree = stem->getParameterTree();
	string name = this->nodeValue->currentText().toStdString();
	tree.addSibling(name);

	int size;
	size_t index = name.rfind('.');
	if (index != string::npos) {
		size = stoi(name.substr(index+1, name.size()-index-1));
		name.erase(index+1, name.size()-index-1);
	} else {
		size = stoi(name);
		name.clear();
	}

	setFields(tree, name + std::to_string(size+1));
	for (auto it = instances.begin(); it != instances.end(); it++)
		it->first->setParameterTree(tree);

	changeOnce();
	emit parameterTreeModified();
}

void GeneratorEditor::removeNode()
{
	auto instances = this->editor->getSelection()->getStemInstances();
	if (instances.empty())
		return;

	beginChanging();
	Stem *stem = instances.begin()->first;
	ParameterTree tree = stem->getParameterTree();
	string name = this->nodeValue->currentText().toStdString();
	if (this->nodeValue->currentIndex() == 0)
		tree.reset();
	else
		tree.remove(name);

	setFields(tree, "");
	for (auto it = instances.begin(); it != instances.end(); it++)
		it->first->setParameterTree(tree);

	changeOnce();
	emit parameterTreeModified();
}

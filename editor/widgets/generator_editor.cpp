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
	QWidget(parent), editor(editor), generate(nullptr)
{
	createInterface();
	setEnabled(false);
	connect(this->editor, SIGNAL(selectionChanged()),
		this, SLOT(setFields()));
}

QSize GeneratorEditor::sizeHint() const
{
	return QSize(UI_WIDGET_WIDTH, UI_WIDGET_HEIGHT);
}

inline QGroupBox *createGroup(const char *name)
{
	QGroupBox *group = new QGroupBox(name);
	group->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	return group;
}

inline QFormLayout *createForm(QGroupBox *group)
{
	QFormLayout *form = new QFormLayout(group);
	form->setSizeConstraint(QLayout::SetMinimumSize);
	form->setSpacing(0);
	form->setMargin(0);
	form->setSpacing(UI_FORM_SPACING);
	form->setMargin(UI_FORM_MARGIN);
	return form;
}

void GeneratorEditor::createInterface()
{
	QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
	layout->setMargin(0);
	layout->setSpacing(0);
	createNodeGroup(layout);
	createRootGroup(layout);
	QFormLayout *form;

	for (int i = 0; i < DSize; i++) {
		this->dv[i] = new DoubleSpinBox(this);
		this->dv[i]->setFocusPolicy(Qt::StrongFocus);
		this->dv[i]->setSingleStep(0.1);
		connect(this->dv[i],
			QOverload<double>::of(&DoubleSpinBox::valueChanged),
			this, &GeneratorEditor::change);
		connect(this->dv[i], &DoubleSpinBox::editingFinished,
			this, &GeneratorEditor::finishChanging);
	}
	for (int i = 0; i < ISize; i++) {
		this->iv[i] = new SpinBox(this);
		this->iv[i]->setFocusPolicy(Qt::StrongFocus);
		this->iv[i]->setSingleStep(1);
		connect(this->iv[i],
			QOverload<int>::of(&SpinBox::valueChanged),
			this, &GeneratorEditor::change);
		connect(this->iv[i], &SpinBox::editingFinished,
			this, &GeneratorEditor::finishChanging);
	}

	this->stemGroup = createGroup("Stems");
	form = createForm(this->stemGroup);
	form->addRow("Stems/Unit", this->dv[StemDensity]);
	form->addRow("Start", this->dv[StemStart]);
	form->addRow("Scale", this->dv[Scale]);
	this->dv[Scale]->setSingleStep(0.001);
	this->dv[Scale]->setDecimals(3);
	form->addRow("Angle Variation", this->dv[AngleVariation]);
	form->addRow("Radius Threshold", this->dv[RadiusThreshold]);
	this->dv[RadiusThreshold]->setSingleStep(0.001);
	this->dv[RadiusThreshold]->setDecimals(3);
	form->addRow("Length", this->dv[Length]);
	this->dv[Length]->setSingleStep(1.0f);
	this->dv[Length]->setRange(0, std::numeric_limits<float>::max());
	setFormLayout(form);
	layout->addWidget(this->stemGroup);

	this->leafGroup = createGroup("Leaves");
	form = createForm(this->leafGroup);
	form->addRow("Nodes/Unit", this->dv[LeafDensity]);
	form->addRow("Distance", this->dv[LeafDistance]);
	form->addRow("Rotation", this->dv[LeafRotation]);
	this->dv[LeafRotation]->setRange(
		std::numeric_limits<float>::min(),
		std::numeric_limits<float>::max());
	form->addRow("Min Up", this->dv[MinUp]);
	form->addRow("Max Up", this->dv[MaxUp]);
	form->addRow("Min Direction", this->dv[MinDirection]);
	form->addRow("Max Direction", this->dv[MaxDirection]);
	form->addRow("Leaves/Node", this->iv[LeavesPerNode]);
	form->addRow("Scale.X", this->dv[ScaleX]);
	form->addRow("Scale.Y", this->dv[ScaleY]);
	form->addRow("Scale.Z", this->dv[ScaleZ]);
	setFormLayout(form);
	layout->addWidget(this->leafGroup);
}

void GeneratorEditor::createNodeGroup(QBoxLayout *layout)
{
	this->nodeGroup = createGroup("Nodes");
	QFormLayout *form = createForm(this->nodeGroup);

	this->nodeValue = new ComboBox(this);
	form->addRow(this->nodeValue);
	this->childButton = new QPushButton("Add Child Node", this);
	form->addRow(this->childButton);
	this->siblingButton = new QPushButton("Add Sibling Node", this);
	form->addRow(this->siblingButton);
	this->removeButton = new QPushButton("Remove Node", this);
	form->addRow(this->removeButton);

	connect(this->nodeValue,
		QOverload<int>::of(&ComboBox::currentIndexChanged),
		this, &GeneratorEditor::select);
	connect(this->childButton, &QPushButton::clicked,
		this, &GeneratorEditor::addChildNode);
	connect(this->siblingButton, &QPushButton::clicked,
		this, &GeneratorEditor::addSiblingNode);
	connect(this->removeButton, &QPushButton::clicked,
		this, &GeneratorEditor::removeNode);

	setFormLayout(form);
	layout->addWidget(this->nodeGroup);
}

void GeneratorEditor::createRootGroup(QBoxLayout *layout)
{
	this->rootGroup = createGroup("Root");
	QFormLayout *form = createForm(this->rootGroup);

	this->seedValue = new SpinBox(this);
	this->seedValue->setRange(
		std::numeric_limits<int>::min(),
		std::numeric_limits<int>::max());
	form->addRow("Seed", this->seedValue);

	connect(this->seedValue,
		QOverload<int>::of(&SpinBox::valueChanged),
		this, &GeneratorEditor::change);
	connect(this->seedValue, &SpinBox::editingFinished,
		this, &GeneratorEditor::finishChanging);

	setFormLayout(form);
	layout->addWidget(this->rootGroup);
}

void GeneratorEditor::setFields()
{
	setEnabled(false);
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
	setEnabled(true);
	blockSignals(false);
}

void GeneratorEditor::setStemData(pg::StemData data)
{
	this->dv[StemDensity]->setValue(data.density);
	this->dv[StemStart]->setValue(data.start);
	this->dv[Length]->setValue(data.length);
	this->dv[Scale]->setValue(data.scale);
	this->dv[AngleVariation]->setValue(data.angleVariation);
	this->dv[RadiusThreshold]->setValue(data.radiusThreshold);
	setLeafData(data.leaf);
}

void GeneratorEditor::setLeafData(pg::LeafData data)
{
	this->dv[LeafDensity]->setValue(data.density);
	this->dv[LeafDistance]->setValue(data.distance);
	this->dv[LeafRotation]->setValue(data.rotation/pi*180.0f);
	this->dv[MinUp]->setValue(data.minUp);
	this->dv[MaxUp]->setValue(data.maxUp);
	this->dv[MinDirection]->setValue(data.minDirection);
	this->dv[MaxDirection]->setValue(data.maxDirection);
	this->dv[ScaleX]->setValue(data.scale.x);
	this->dv[ScaleY]->setValue(data.scale.y);
	this->dv[ScaleZ]->setValue(data.scale.z);
	this->iv[LeavesPerNode]->setValue(data.leavesPerNode);
}

void GeneratorEditor::setEnabled(bool enable)
{
	this->seedValue->setEnabled(enable);
	this->nodeValue->setEnabled(enable);
	this->childButton->setEnabled(enable);
	this->siblingButton->setEnabled(enable);
	this->removeButton->setEnabled(enable);

	if (this->nodeValue->currentIndex() == 0)
		enable = false;

	for (int i = 0; i < DSize; i++)
		this->dv[i]->setEnabled(enable);
	for (int i = 0; i < ISize; i++)
		this->iv[i]->setEnabled(enable);
}

void GeneratorEditor::blockSignals(bool block)
{
	this->seedValue->blockSignals(block);
	this->nodeValue->blockSignals(block);
	for (int i = 0; i < DSize; i++)
		this->dv[i]->blockSignals(block);
	for (int i = 0; i < ISize; i++)
		this->iv[i]->blockSignals(block);
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
	data.density = this->dv[StemDensity]->value();
	data.start = this->dv[StemStart]->value();
	data.length = this->dv[Length]->value();
	data.scale = this->dv[Scale]->value();
	data.angleVariation = this->dv[AngleVariation]->value();
	data.radiusThreshold = this->dv[RadiusThreshold]->value();
	data.leaf = getLeafData(data.leaf);
	return data;
}

pg::LeafData GeneratorEditor::getLeafData(LeafData data)
{
	data.density = this->dv[LeafDensity]->value();
	data.distance = this->dv[LeafDistance]->value();
	data.rotation = this->dv[LeafRotation]->value()/180.0f*pi;
	data.minUp = this->dv[MinUp]->value();
	data.maxUp = this->dv[MaxUp]->value();
	data.minDirection = this->dv[MinDirection]->value();
	data.maxDirection = this->dv[MaxDirection]->value();
	data.scale.x = this->dv[ScaleX]->value();
	data.scale.y = this->dv[ScaleY]->value();
	data.scale.z = this->dv[ScaleZ]->value();
	data.leavesPerNode = this->iv[LeavesPerNode]->value();
	return data;
}

void GeneratorEditor::changeOnce()
{
	change();
	finishChanging();
}

void GeneratorEditor::beginChanging()
{
	if (!this->generate) {
		Selection *selection = this->editor->getSelection();
		pg::Scene *scene = this->editor->getScene();
		this->generate = new Generate(selection, &scene->generator);
	}
}

void GeneratorEditor::finishChanging()
{
	if (this->generate) {
		this->editor->getHistory()->add(this->generate);
		this->generate = nullptr;
	}
}

void GeneratorEditor::select()
{
	auto instances = this->editor->getSelection()->getStemInstances();
	if (!instances.empty()) {
		Stem *stem = instances.begin()->first;
		ParameterTree tree = stem->getParameterTree();
		setFields(tree, this->nodeValue->currentText().toStdString());
	}
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

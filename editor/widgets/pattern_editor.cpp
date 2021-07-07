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

#include "pattern_editor.h"
#include "form.h"
#include <limits>

using pg::PatternGenerator;
using pg::ParameterTree;
using pg::ParameterNode;
using pg::LeafData;
using pg::StemData;
using pg::Stem;
using pg::Spline;
using std::string;

const float pi = 3.14159265359f;

PatternEditor::PatternEditor(SharedResources *shared, KeyMap *keymap,
	Editor *editor, QWidget *parent) :
	QWidget(parent),
	editor(editor),
	keymap(keymap),
	shared(shared),
	generate(nullptr)
{
	createInterface();
	setEnabled(false);
	connect(this->editor, SIGNAL(selectionChanged()),
		this, SLOT(setFields()));
}

QSize PatternEditor::sizeHint() const
{
	return QSize(UI_WIDGET_WIDTH, UI_WIDGET_HEIGHT);
}

void PatternEditor::createInterface()
{
	QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
	layout->setMargin(0);
	layout->setSpacing(0);
	createNodeGroup(layout);
	createRootGroup(layout);
	createStemLeafFields();
	createStemGroup(layout);
	createLeafGroup(layout);
	createCurveGroup(layout);
	layout->addStretch(1);
}

void PatternEditor::createNodeGroup(QBoxLayout *layout)
{
	QGroupBox *nodeGroup = createGroup("Nodes");
	QFormLayout *form = createForm(nodeGroup);

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
		this, &PatternEditor::select);
	connect(this->childButton, &QPushButton::clicked,
		this, &PatternEditor::addChildNode);
	connect(this->siblingButton, &QPushButton::clicked,
		this, &PatternEditor::addSiblingNode);
	connect(this->removeButton, &QPushButton::clicked,
		this, &PatternEditor::removeNode);

	setFormLayout(form);
	layout->addWidget(nodeGroup);
}

void PatternEditor::createRootGroup(QBoxLayout *layout)
{
	QGroupBox *rootGroup = createGroup("Root");
	QFormLayout *form = createForm(rootGroup);

	for (int i = 0; i < DRSize; i++) {
		this->drv[i] = new DoubleSpinBox(this);
		this->drv[i]->setSingleStep(0.1);
		connect(this->drv[i],
			QOverload<double>::of(&DoubleSpinBox::valueChanged),
			this, &PatternEditor::change);
	}
	for (int i = 0; i < IRSize; i++) {
		this->irv[i] = new SpinBox(this);
		this->irv[i]->setSingleStep(1);
		connect(this->irv[i],
			QOverload<int>::of(&SpinBox::valueChanged),
			this, &PatternEditor::change);
	}

	const int min = std::numeric_limits<int>::min();
	const int max = std::numeric_limits<int>::max();

	this->irv[Seed]->setRange(min, max);
	form->addRow("Seed", this->irv[Seed]);
	form->addRow("Length", this->drv[RootLength]);
	form->addRow("Noise", this->drv[RootNoise]);
	this->drv[RootNoise]->setSingleStep(0.001);
	this->drv[RootNoise]->setDecimals(3);
	form->addRow("Bifurcation", this->drv[RootFork]);
	this->drv[RootFork]->setSingleStep(0.01);
	this->drv[RootFork]->setDecimals(2);
	form->addRow("Bifurcation Angle", this->drv[RootForkAngle]);
	this->drv[RootForkAngle]->setSingleStep(0.01);
	this->drv[RootForkAngle]->setDecimals(2);
	form->addRow("Radius Threshold", this->drv[RootThreshold]);
	this->drv[RootThreshold]->setSingleStep(0.001);
	this->drv[RootThreshold]->setDecimals(3);

	setFormLayout(form);
	layout->addWidget(rootGroup);
}

void PatternEditor::createStemLeafFields()
{
	for (int i = 0; i < DSize; i++) {
		this->dv[i] = new DoubleSpinBox(this);
		this->dv[i]->setFocusPolicy(Qt::StrongFocus);
		this->dv[i]->setSingleStep(0.1);
		connect(this->dv[i],
			QOverload<double>::of(&DoubleSpinBox::valueChanged),
			this, &PatternEditor::change);
	}
	for (int i = 0; i < ISize; i++) {
		this->iv[i] = new SpinBox(this);
		this->iv[i]->setFocusPolicy(Qt::StrongFocus);
		this->iv[i]->setSingleStep(1);
		connect(this->iv[i],
			QOverload<int>::of(&SpinBox::valueChanged),
			this, &PatternEditor::change);
	}
}

void PatternEditor::createStemGroup(QBoxLayout *layout)
{
	QGroupBox *stemGroup = createGroup("Stems");
	QFormLayout *form = createForm(stemGroup);
	form->addRow("Stem Density", this->dv[StemDensity]);
	form->addRow("Length", this->dv[Length]);
	this->dv[Length]->setSingleStep(1.0);
	this->dv[Length]->setRange(0.0, std::numeric_limits<float>::max());
	form->addRow("Distance", this->dv[StemDistance]);
	form->addRow("Noise", this->dv[Noise]);
	this->dv[Noise]->setSingleStep(0.001);
	this->dv[Noise]->setDecimals(3);
	form->addRow("Bifurcation", this->dv[Fork]);
	this->dv[Fork]->setSingleStep(0.01);
	this->dv[Fork]->setDecimals(2);
	form->addRow("Bifurcation Angle", this->dv[ForkAngle]);
	this->dv[ForkAngle]->setSingleStep(0.01);
	this->dv[ForkAngle]->setDecimals(2);
	form->addRow("Angle Variation", this->dv[AngleVariation]);
	form->addRow("Radius Threshold", this->dv[RadiusThreshold]);
	this->dv[RadiusThreshold]->setSingleStep(0.001);
	this->dv[RadiusThreshold]->setDecimals(3);
	form->addRow("Radius Scale", this->dv[Scale]);
	this->dv[Scale]->setSingleStep(0.001);
	this->dv[Scale]->setDecimals(3);
	setFormLayout(form);
	layout->addWidget(stemGroup);
}

void PatternEditor::createLeafGroup(QBoxLayout *layout)
{
	QGroupBox *leafGroup = createGroup("Leaves");
	QFormLayout *form = createForm(leafGroup);
	form->addRow("Node Density", this->dv[LeafDensity]);
	form->addRow("Distance", this->dv[LeafDistance]);
	form->addRow("Rotation", this->dv[LeafRotation]);
	const float min = std::numeric_limits<float>::lowest();
	const float max = std::numeric_limits<float>::max();
	this->dv[LeafRotation]->setRange(min, max);
	form->addRow("Min Up", this->dv[MinUp]);
	this->dv[MinUp]->setRange(min, max);
	form->addRow("Max Up", this->dv[MaxUp]);
	this->dv[MaxUp]->setRange(min, max);
	form->addRow("Local Up", this->dv[LocalUp]);
	this->dv[LocalUp]->setRange(min, max);
	form->addRow("Global Up", this->dv[GlobalUp]);
	this->dv[GlobalUp]->setRange(min, max);
	form->addRow("Min Forward", this->dv[MinForward]);
	this->dv[MinForward]->setRange(min, max);
	form->addRow("Max Forward", this->dv[MaxForward]);
	this->dv[MaxForward]->setRange(min, max);
	form->addRow("Vertical Pull", this->dv[Pull]);
	this->dv[Pull]->setRange(min, max);
	form->addRow("Leaves/Node", this->iv[LeavesPerNode]);
	form->addRow("Scale.X", this->dv[ScaleX]);
	form->addRow("Scale.Y", this->dv[ScaleY]);
	form->addRow("Scale.Z", this->dv[ScaleZ]);
	setFormLayout(form);
	layout->addWidget(leafGroup);
}

void PatternEditor::setFields()
{
	setEnabled(false);
	setCurveFields();
	auto instances = this->editor->getSelection()->getStemInstances();
	if (!instances.empty()) {
		ParameterTree tree;
		tree = instances.begin()->first->getParameterTree();
		ParameterNode *root = tree.getRoot();
		if (root && root->getChild())
			setFields(tree, "1");
		else
			setFields(tree, "");
	}
}

void PatternEditor::setFields(const ParameterTree &tree, string name)
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

		StemData data = tree.getRoot()->getData();
		this->irv[Seed]->setValue(data.seed);
		this->drv[RootLength]->setValue(data.length);
		this->drv[RootFork]->setValue(data.fork);
		this->drv[RootForkAngle]->setValue(data.forkAngle);
		this->drv[RootNoise]->setValue(data.noise);
		this->drv[RootThreshold]->setValue(data.radiusThreshold);
	} else {
		for (int i = 0; i < DRSize; i++)
			this->drv[i]->setValue(0);
		for (int i = 0; i < IRSize; i++)
			this->irv[i]->setValue(0.0);
	}
	if (node) {
		this->nodeValue->setCurrentText(QString::fromStdString(name));
		setStemData(node->getData());
	}
	setEnabled(true);
	blockSignals(false);
}

void PatternEditor::setStemData(StemData data)
{
	this->dv[StemDensity]->setValue(data.density);
	this->dv[StemDistance]->setValue(data.distance);
	this->dv[Length]->setValue(data.length);
	this->dv[Fork]->setValue(data.fork);
	this->dv[ForkAngle]->setValue(data.forkAngle);
	this->dv[Noise]->setValue(data.noise);
	this->dv[Scale]->setValue(data.scale);
	this->dv[AngleVariation]->setValue(data.angleVariation);
	this->dv[RadiusThreshold]->setValue(data.radiusThreshold);
	setLeafData(data.leaf);
}

void PatternEditor::setLeafData(LeafData data)
{
	this->dv[LeafDensity]->setValue(data.density);
	this->dv[LeafDistance]->setValue(data.distance);
	this->dv[LeafRotation]->setValue(data.rotation/pi*180.0f);
	this->dv[MinUp]->setValue(data.minUp);
	this->dv[MaxUp]->setValue(data.maxUp);
	this->dv[LocalUp]->setValue(data.localUp);
	this->dv[GlobalUp]->setValue(data.globalUp);
	this->dv[MinForward]->setValue(data.minForward);
	this->dv[MaxForward]->setValue(data.maxForward);
	this->dv[Pull]->setValue(data.verticalPull);
	this->dv[ScaleX]->setValue(data.scale.x);
	this->dv[ScaleY]->setValue(data.scale.y);
	this->dv[ScaleZ]->setValue(data.scale.z);
	this->iv[LeavesPerNode]->setValue(data.leavesPerNode);
}

void PatternEditor::setEnabled(bool enable)
{
	this->nodeValue->setEnabled(enable);
	this->childButton->setEnabled(enable);
	this->siblingButton->setEnabled(enable);
	this->removeButton->setEnabled(enable);
	for (int i = 0; i < DRSize; i++)
		this->drv[i]->setEnabled(enable);
	for (int i = 0; i < IRSize; i++)
		this->irv[i]->setEnabled(enable);
	if (this->nodeValue->currentIndex() == 0)
		enable = false;
	for (int i = 0; i < DSize; i++)
		this->dv[i]->setEnabled(enable);
	for (int i = 0; i < ISize; i++)
		this->iv[i]->setEnabled(enable);
}

void PatternEditor::blockSignals(bool block)
{
	this->nodeValue->blockSignals(block);
	for (int i = 0; i < DRSize; i++)
		this->drv[i]->blockSignals(block);
	for (int i = 0; i < IRSize; i++)
		this->irv[i]->blockSignals(block);
	for (int i = 0; i < DSize; i++)
		this->dv[i]->blockSignals(block);
	for (int i = 0; i < ISize; i++)
		this->iv[i]->blockSignals(block);
}

void PatternEditor::change()
{
	auto instances = this->editor->getSelection()->getStemInstances();
	if (instances.empty())
		return;

	createCommand();
	Stem *stem = instances.begin()->first;
	ParameterTree tree = stem->getParameterTree();
	if (!tree.getRoot())
		tree.createRoot();

	StemData data = getRootData(tree.getRoot()->getData());
	tree.getRoot()->setData(data);

	if (this->nodeValue->currentIndex() > 0) {
		string name = this->nodeValue->currentText().toStdString();
		ParameterNode *node = tree.get(name);
		data = getStemData(node->getData());
		node->setData(data);
	}

	for (auto instance : instances)
		instance.first->setParameterTree(tree);

	this->generate->execute();
	this->editor->change();
}

StemData PatternEditor::getRootData(StemData data)
{
	data.seed = this->irv[Seed]->value();
	data.length = this->drv[RootLength]->value();
	data.fork = this->drv[RootFork]->value();
	data.forkAngle = this->drv[RootForkAngle]->value();
	data.radiusThreshold = this->drv[RootThreshold]->value();
	data.noise = this->drv[RootNoise]->value();
	return data;
}

StemData PatternEditor::getStemData(StemData data)
{
	data.density = this->dv[StemDensity]->value();
	data.distance = this->dv[StemDistance]->value();
	data.length = this->dv[Length]->value();
	data.fork = this->dv[Fork]->value();
	data.forkAngle = this->dv[ForkAngle]->value();
	data.noise = this->dv[Noise]->value();
	data.scale = this->dv[Scale]->value();
	data.angleVariation = this->dv[AngleVariation]->value();
	data.radiusThreshold = this->dv[RadiusThreshold]->value();
	data.leaf = getLeafData(data.leaf);
	return data;
}

LeafData PatternEditor::getLeafData(LeafData data)
{
	data.density = this->dv[LeafDensity]->value();
	data.distance = this->dv[LeafDistance]->value();
	data.rotation = this->dv[LeafRotation]->value()/180.0f*pi;
	data.minUp = this->dv[MinUp]->value();
	data.maxUp = this->dv[MaxUp]->value();
	data.localUp = this->dv[LocalUp]->value();
	data.globalUp = this->dv[GlobalUp]->value();
	data.minForward = this->dv[MinForward]->value();
	data.maxForward = this->dv[MaxForward]->value();
	data.verticalPull = this->dv[Pull]->value();
	data.scale.x = this->dv[ScaleX]->value();
	data.scale.y = this->dv[ScaleY]->value();
	data.scale.z = this->dv[ScaleZ]->value();
	data.leavesPerNode = this->iv[LeavesPerNode]->value();
	return data;
}

void PatternEditor::createCommand()
{
	History *history = this->editor->getHistory();
	const Command *command = history->peak();
	bool undefined = !this->generate || !command;
	if (undefined || command->getTime() != this->generate->getTime()) {
		Selection *selection = this->editor->getSelection();
		pg::Scene *scene = this->editor->getScene();
		this->generate = new Generate(selection, &scene->pattern);
		history->add(this->generate);
	}
}

void PatternEditor::select()
{
	auto instances = this->editor->getSelection()->getStemInstances();
	if (!instances.empty()) {
		Stem *stem = instances.begin()->first;
		ParameterTree tree = stem->getParameterTree();
		setFields(tree, this->nodeValue->currentText().toStdString());
	}
}

void PatternEditor::addChildNode()
{
	auto instances = this->editor->getSelection()->getStemInstances();
	if (instances.empty())
		return;

	createCommand();
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

	setCurveFields();
	change();
}

void PatternEditor::addSiblingNode()
{
	auto instances = this->editor->getSelection()->getStemInstances();
	if (instances.empty() || this->nodeValue->currentIndex() <= 0)
		return;

	createCommand();
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

	setCurveFields();
	change();
}

void PatternEditor::removeNode()
{
	auto instances = this->editor->getSelection()->getStemInstances();
	if (instances.empty())
		return;

	createCommand();
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

	setCurveFields();
	change();
}

void PatternEditor::createCurveGroup(QBoxLayout *layout)
{
	QGroupBox *curveGroup = createGroup("Curve");
	layout->addWidget(curveGroup);
	QBoxLayout *vlayout = new QVBoxLayout(curveGroup);
	vlayout->setMargin(0);
	vlayout->setSpacing(0);
	this->curveEditor = new CurveEditor(this->shared, this->keymap, this);
	vlayout->addWidget(this->curveEditor);
	QFormLayout *form = createForm(vlayout);
	this->curveDegree = new ComboBox();
	this->curveDegree->addItem("Linear");
	this->curveDegree->addItem("Cubic");
	this->curveType = new ComboBox();
	this->curveType->addItem("Stem Density");
	this->curveType->addItem("Leaf Density");
	this->curveNode = new ComboBox();
	form->addRow("Degree", this->curveDegree);
	form->addRow("Type", this->curveType);
	form->addRow("Node", this->curveNode);
	setFormLayout(form);

	connect(this->curveDegree,
		QOverload<int>::of(&ComboBox::currentIndexChanged),
		this->curveEditor, &CurveEditor::setDegree);
	connect(this->curveType,
		QOverload<int>::of(&ComboBox::currentIndexChanged),
		this, &PatternEditor::selectCurve);
	connect(this->curveNode,
		QOverload<int>::of(&ComboBox::currentIndexChanged),
		this, &PatternEditor::selectCurve);

	this->curveEditor->setUpdateFunction(std::bind(
		&PatternEditor::updateCurve, this, std::placeholders::_1));
}

void PatternEditor::setCurveFields()
{
	auto instances = this->editor->getSelection()->getStemInstances();
	if (instances.empty()) {
		this->curveEditor->clear();
		enableCurve(false);
		return;
	}

	Stem *stem = instances.begin()->first;
	ParameterTree tree = stem->getParameterTree();
	std::vector<string> names = tree.getNames();
	this->curveNode->clear();
	this->curveNode->addItem("");
	for (string name : names)
		this->curveNode->addItem(QString::fromStdString(name));
	if (!names.empty())
		this->curveNode->setCurrentIndex(1);

	selectCurve();
}

void PatternEditor::selectCurve()
{
	this->curveEditor->clear();
	auto instances = this->editor->getSelection()->getStemInstances();
	if (instances.empty())
		return;

	int degree = 0;
	int index = this->curveType->currentIndex();
	Stem *stem = instances.begin()->first;
	ParameterTree tree = stem->getParameterTree();

	if (this->curveNode->currentIndex() > 0) {
		string name = this->curveNode->currentText().toStdString();
		ParameterNode *node = tree.get(name);
		if (index == 0) {
			Spline spline = node->getData().densityCurve;
			degree = spline.getDegree();
			this->curveEditor->setSpline(spline);
		} else {
			Spline spline = node->getData().leaf.densityCurve;
			degree = spline.getDegree();
			this->curveEditor->setSpline(spline);
		}
	}

	this->curveDegree->blockSignals(true);
	this->curveDegree->setCurrentIndex(degree == 3 ? 1 : 0);
	this->curveDegree->blockSignals(false);
	enableCurve(true);
}

void PatternEditor::enableCurve(bool enable)
{
	this->curveDegree->setEnabled(enable);
	this->curveNode->setEnabled(enable);
	this->curveType->setEnabled(enable);
}

void PatternEditor::updateCurve(Spline spline)
{
	createCommand();
	updateParameterTree(spline);
	this->generate->execute();
	this->editor->change();
}

void PatternEditor::updateParameterTree(Spline spline)
{
	string name = this->curveNode->currentText().toStdString();
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto instance : instances) {
		int index = this->curveType->currentIndex();
		Stem *stem = instance.first;
		ParameterTree tree = stem->getParameterTree();
		if (!tree.getRoot())
			continue;

		if (this->curveNode->currentIndex() > 0) {
			ParameterNode *node = tree.get(name);
			if (!node)
				continue;
			StemData data = node->getData();
			if (index == 0)
				data.densityCurve = spline;
			else
				data.leaf.densityCurve = spline;
			node->setData(data);
			stem->setParameterTree(tree);
		}
	}
}

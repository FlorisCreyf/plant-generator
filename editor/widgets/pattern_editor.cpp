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
	QBoxLayout *layout = new QVBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(0);
	createNodeGroup(layout);
	createRootGroup(layout);
	createStemGroup(layout);
	createLeafGroup(layout);
	createCurveGroup(layout);
	layout->addStretch(1);
}

void PatternEditor::createNodeGroup(QBoxLayout *layout)
{
	QGroupBox *nodeGroup = createGroup("Nodes");
	layout->addWidget(nodeGroup);
	QFormLayout *form = createForm(nodeGroup);

	this->nodeValue = new ComboBox(this);
	form->addRow(this->nodeValue);
	this->childButton = new QPushButton("Add Child Node", this);
	form->addRow(this->childButton);
	this->siblingButton = new QPushButton("Add Sibling Node", this);
	form->addRow(this->siblingButton);
	this->removeButton = new QPushButton("Remove Node", this);
	form->addRow(this->removeButton);
	setFormLayout(form);

	connect(this->nodeValue,
		QOverload<int>::of(&ComboBox::currentIndexChanged),
		this, &PatternEditor::select);
	connect(this->childButton, &QPushButton::clicked,
		this, &PatternEditor::addChildNode);
	connect(this->siblingButton, &QPushButton::clicked,
		this, &PatternEditor::addSiblingNode);
	connect(this->removeButton, &QPushButton::clicked,
		this, &PatternEditor::removeNode);
}

void PatternEditor::createRootGroup(QBoxLayout *layout)
{
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

	QGroupBox *rootGroup = createGroup("Root");
	QFormLayout *form = createForm(rootGroup);
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

void PatternEditor::createStemGroup(QBoxLayout *layout)
{
	std::function<void(StemData *)> f[DSSize] = {
		[&] (StemData *d) {d->density =
			this->dsv[StemDensity]->value();},
		[&] (StemData *d) {d->distance =
			this->dsv[StemDistance]->value();},
		[&] (StemData *d) {d->length =
			this->dsv[Length]->value();},
		[&] (StemData *d) {d->radius =
			this->dsv[Radius]->value();},
		[&] (StemData *d) {d->angleVariation =
			this->dsv[AngleVariation]->value();},
		[&] (StemData *d) {d->noise =
			this->dsv[Noise]->value();},
		[&] (StemData *d) {d->radiusThreshold =
			this->dsv[RadiusThreshold]->value();},
		[&] (StemData *d) {d->fork =
			this->dsv[Fork]->value();},
		[&] (StemData *d) {d->forkAngle =
			this->dsv[ForkAngle]->value();},
		[&] (StemData *d) {d->inclineVariation =
			this->dsv[InclineVariation]->value();},
		[&] (StemData *d) {d->gravity =
			this->dsv[StemPull]->value();},
		[&] (StemData *d) {d->radiusVariation =
			this->dsv[RadiusVariation]->value();}
	};

	for (int i = 0; i < DSSize; i++) {
		this->dsv[i] = new DoubleSpinBox(this);
		this->dsv[i]->setSingleStep(0.1);
		connect(this->dsv[i],
			QOverload<double>::of(&DoubleSpinBox::valueChanged),
			std::bind(&PatternEditor::changeField, this, f[i]));
	}

	QGroupBox *stemGroup = createGroup("Stems");
	QFormLayout *form = createForm(stemGroup);
	form->addRow("Stem Density", this->dsv[StemDensity]);
	form->addRow("Length", this->dsv[Length]);
	this->dsv[Length]->setSingleStep(1.0);
	this->dsv[Length]->setRange(0.0, std::numeric_limits<float>::max());
	form->addRow("Distance", this->dsv[StemDistance]);
	form->addRow("Noise", this->dsv[Noise]);
	this->dsv[Noise]->setSingleStep(0.001);
	this->dsv[Noise]->setDecimals(3);
	form->addRow("Bifurcation", this->dsv[Fork]);
	this->dsv[Fork]->setSingleStep(0.01);
	this->dsv[Fork]->setDecimals(2);
	form->addRow("Bifurcation Angle", this->dsv[ForkAngle]);
	this->dsv[ForkAngle]->setSingleStep(0.01);
	this->dsv[ForkAngle]->setDecimals(2);
	form->addRow("Angle Variation", this->dsv[AngleVariation]);
	form->addRow("Incline Variation", this->dsv[InclineVariation]);
	form->addRow("Gravity", this->dsv[StemPull]);
	form->addRow("Radius Threshold", this->dsv[RadiusThreshold]);
	this->dsv[RadiusThreshold]->setSingleStep(0.001);
	this->dsv[RadiusThreshold]->setDecimals(3);
	form->addRow("Radius Variation", this->dsv[RadiusVariation]);
	form->addRow("Radius", this->dsv[Radius]);
	this->dsv[Radius]->setSingleStep(0.001);
	this->dsv[Radius]->setDecimals(3);
	setFormLayout(form);
	layout->addWidget(stemGroup);
}

void PatternEditor::createLeafGroup(QBoxLayout *layout)
{
	std::function<void(StemData *)> fd[DLSize] = {
		[&] (StemData *d) {d->leaf.density =
			this->dlv[LeafDensity]->value();},
		[&] (StemData *d) {d->leaf.distance =
			this->dlv[LeafDistance]->value();},
		[&] (StemData *d) {d->leaf.rotation =
			this->dlv[LeafRotation]->value();},
		[&] (StemData *d) {d->leaf.minUp =
			this->dlv[MinUp]->value();},
		[&] (StemData *d) {d->leaf.maxUp =
			this->dlv[MaxUp]->value();},
		[&] (StemData *d) {d->leaf.localUp =
			this->dlv[LocalUp]->value();},
		[&] (StemData *d) {d->leaf.globalUp =
			this->dlv[GlobalUp]->value();},
		[&] (StemData *d) {d->leaf.minForward =
			this->dlv[MinForward]->value();},
		[&] (StemData *d) {d->leaf.maxForward =
			this->dlv[MaxForward]->value();},
		[&] (StemData *d) {d->leaf.gravity =
			this->dlv[LeafPull]->value();},
		[&] (StemData *d) {d->leaf.scale.x =
			this->dlv[ScaleX]->value();},
		[&] (StemData *d) {d->leaf.scale.y =
			this->dlv[ScaleY]->value();},
		[&] (StemData *d) {d->leaf.scale.z =
			this->dlv[ScaleZ]->value();}
	};
	std::function<void(StemData *)> fi[ILSize] = {
		[&] (StemData *d) {d->leaf.leavesPerNode =
			this->ilv[LeavesPerNode]->value();}
	};

	for (int i = 0; i < DLSize; i++) {
		this->dlv[i] = new DoubleSpinBox(this);
		this->dlv[i]->setSingleStep(0.1);
		connect(this->dlv[i],
			QOverload<double>::of(&DoubleSpinBox::valueChanged),
			std::bind(&PatternEditor::changeField, this, fd[i]));
	}
	for (int i = 0; i < ILSize; i++) {
		this->ilv[i] = new SpinBox(this);
		this->ilv[i]->setSingleStep(1);
		connect(this->ilv[i],
			QOverload<int>::of(&SpinBox::valueChanged),
			std::bind(&PatternEditor::changeField, this, fi[i]));
	}

	QGroupBox *leafGroup = createGroup("Leaves");
	QFormLayout *form = createForm(leafGroup);
	form->addRow("Node Density", this->dlv[LeafDensity]);
	form->addRow("Distance", this->dlv[LeafDistance]);
	form->addRow("Rotation", this->dlv[LeafRotation]);
	const float min = std::numeric_limits<float>::lowest();
	const float max = std::numeric_limits<float>::max();
	this->dlv[LeafRotation]->setRange(min, max);
	form->addRow("Min Up", this->dlv[MinUp]);
	this->dlv[MinUp]->setRange(min, max);
	form->addRow("Max Up", this->dlv[MaxUp]);
	this->dlv[MaxUp]->setRange(min, max);
	form->addRow("Local Up", this->dlv[LocalUp]);
	this->dlv[LocalUp]->setRange(min, max);
	form->addRow("Global Up", this->dlv[GlobalUp]);
	this->dlv[GlobalUp]->setRange(min, max);
	form->addRow("Min Forward", this->dlv[MinForward]);
	this->dlv[MinForward]->setRange(min, max);
	form->addRow("Max Forward", this->dlv[MaxForward]);
	this->dlv[MaxForward]->setRange(min, max);
	form->addRow("Gravity", this->dlv[LeafPull]);
	this->dlv[LeafPull]->setRange(min, max);
	form->addRow("Leaves/Node", this->ilv[LeavesPerNode]);
	form->addRow("Scale.X", this->dlv[ScaleX]);
	form->addRow("Scale.Y", this->dlv[ScaleY]);
	form->addRow("Scale.Z", this->dlv[ScaleZ]);
	setFormLayout(form);
	layout->addWidget(leafGroup);
}

void PatternEditor::setFields()
{
	setEnabled(false);
	setCurveFields();
	auto instances = this->editor->getSelection()->getStemInstances();
	if (!instances.empty()) {
		Stem *stem = instances.begin()->first;
		ParameterTree tree = stem->getParameterTree();
		ParameterNode *root = tree.getRoot();
		if (tree.get(this->name))
			setFields(tree, this->name);
		else if (root && root->getChild())
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
	for (auto instance : instances)
		instance.first->setParameterTree(tree);

	this->generate->execute();
	this->editor->change();
}

void PatternEditor::changeField(std::function<void(StemData *)> function)
{
	auto instances = this->editor->getSelection()->getStemInstances();
	if (instances.empty())
		return;

	createCommand();
	Stem *stem = instances.begin()->first;
	ParameterTree tree = stem->getParameterTree();

	if (this->nodeValue->currentIndex()) {
		Qt::KeyboardModifiers km = QGuiApplication::keyboardModifiers();
		if (km.testFlag(Qt::ShiftModifier))
			tree.updateFields(function);
		else {
			QString name = this->nodeValue->currentText();
			tree.updateField(function, name.toStdString());
		}
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
	data.density = this->dsv[StemDensity]->value();
	data.distance = this->dsv[StemDistance]->value();
	data.length = this->dsv[Length]->value();
	data.fork = this->dsv[Fork]->value();
	data.forkAngle = this->dsv[ForkAngle]->value();
	data.noise = this->dsv[Noise]->value();
	data.radius = this->dsv[Radius]->value();
	data.angleVariation = this->dsv[AngleVariation]->value();
	data.radiusThreshold = this->dsv[RadiusThreshold]->value();
	data.inclineVariation = this->dsv[InclineVariation]->value();
	data.radiusVariation = this->dsv[RadiusVariation]->value();
	data.gravity = this->dsv[StemPull]->value();
	data.leaf = getLeafData(data.leaf);
	return data;
}

LeafData PatternEditor::getLeafData(LeafData data)
{
	data.density = this->dlv[LeafDensity]->value();
	data.distance = this->dlv[LeafDistance]->value();
	data.rotation = this->dlv[LeafRotation]->value()/180.0f*pi;
	data.minUp = this->dlv[MinUp]->value();
	data.maxUp = this->dlv[MaxUp]->value();
	data.localUp = this->dlv[LocalUp]->value();
	data.globalUp = this->dlv[GlobalUp]->value();
	data.minForward = this->dlv[MinForward]->value();
	data.maxForward = this->dlv[MaxForward]->value();
	data.gravity = this->dlv[LeafPull]->value();
	data.scale.x = this->dlv[ScaleX]->value();
	data.scale.y = this->dlv[ScaleY]->value();
	data.scale.z = this->dlv[ScaleZ]->value();
	data.leavesPerNode = this->ilv[LeavesPerNode]->value();
	return data;
}

void PatternEditor::setStemData(StemData data)
{
	this->dsv[StemDensity]->setValue(data.density);
	this->dsv[StemDistance]->setValue(data.distance);
	this->dsv[Length]->setValue(data.length);
	this->dsv[Fork]->setValue(data.fork);
	this->dsv[ForkAngle]->setValue(data.forkAngle);
	this->dsv[Noise]->setValue(data.noise);
	this->dsv[Radius]->setValue(data.radius);
	this->dsv[AngleVariation]->setValue(data.angleVariation);
	this->dsv[RadiusThreshold]->setValue(data.radiusThreshold);
	this->dsv[InclineVariation]->setValue(data.inclineVariation);
	this->dsv[RadiusVariation]->setValue(data.radiusVariation);
	this->dsv[StemPull]->setValue(data.gravity);
	setLeafData(data.leaf);
}

void PatternEditor::setLeafData(LeafData data)
{
	this->dlv[LeafDensity]->setValue(data.density);
	this->dlv[LeafDistance]->setValue(data.distance);
	this->dlv[LeafRotation]->setValue(data.rotation/pi*180.0f);
	this->dlv[MinUp]->setValue(data.minUp);
	this->dlv[MaxUp]->setValue(data.maxUp);
	this->dlv[LocalUp]->setValue(data.localUp);
	this->dlv[GlobalUp]->setValue(data.globalUp);
	this->dlv[MinForward]->setValue(data.minForward);
	this->dlv[MaxForward]->setValue(data.maxForward);
	this->dlv[LeafPull]->setValue(data.gravity);
	this->dlv[ScaleX]->setValue(data.scale.x);
	this->dlv[ScaleY]->setValue(data.scale.y);
	this->dlv[ScaleZ]->setValue(data.scale.z);
	this->ilv[LeavesPerNode]->setValue(data.leavesPerNode);
}

void PatternEditor::setEnabled(bool enable)
{
	/* Prevent the scrollbar from jumping to the next focused widget. */
	if (QApplication::focusWidget())
		QApplication::focusWidget()->clearFocus();

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
	for (int i = 0; i < DSSize; i++)
		this->dsv[i]->setEnabled(enable);
	for (int i = 0; i < DLSize; i++)
		this->dlv[i]->setEnabled(enable);
	for (int i = 0; i < ILSize; i++)
		this->ilv[i]->setEnabled(enable);
}

void PatternEditor::blockSignals(bool block)
{
	this->nodeValue->blockSignals(block);
	for (int i = 0; i < DRSize; i++)
		this->drv[i]->blockSignals(block);
	for (int i = 0; i < IRSize; i++)
		this->irv[i]->blockSignals(block);
	for (int i = 0; i < DSSize; i++)
		this->dsv[i]->blockSignals(block);
	for (int i = 0; i < DLSize; i++)
		this->dlv[i]->blockSignals(block);
	for (int i = 0; i < ILSize; i++)
		this->ilv[i]->blockSignals(block);
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
		this->name = this->nodeValue->currentText().toStdString();
		setFields(tree, this->name);
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
	this->curveType->addItem("Stem Inclination");
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
		} else if (index == 1) {
			Spline spline = node->getData().inclineCurve;
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
			if (index == 1)
				data.inclineCurve = spline;
			else
				data.leaf.densityCurve = spline;
			node->setData(data);
			stem->setParameterTree(tree);
		}
	}
}

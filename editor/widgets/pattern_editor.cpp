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
	layout->setContentsMargins(0, 0, 0, 0);
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
	QGroupBox *nodeGroup = createGroup("Node");
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
		this->drv[i]->setSingleStep(0.001);
		this->drv[i]->setDecimals(3);
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
	form->addRow("Point Density", this->drv[RPointDensity]);
	this->drv[RPointDensity]->setSingleStep(0.01);
	form->addRow("Length", this->drv[RLength]);
	this->drv[RLength]->setSingleStep(0.01);
	this->drv[RLength]->setRange(0.0, std::numeric_limits<float>::max());
	form->addRow("Noise", this->drv[RNoise]);
	this->drv[RPull]->setRange(min, max);
	form->addRow("Gravity", this->drv[RPull]);
	form->addRow("Bifurcation", this->drv[RFork]);
	form->addRow("Bifurcation Angle", this->drv[RForkAngle]);
	form->addRow("Bifurcation Radius", this->drv[RForkScale]);
	this->irv[RMaxDepth]->setRange(0, 10);
	form->addRow("Radius Threshold", this->drv[RThreshold]);
	form->addRow("Max Depth", this->irv[RMaxDepth]);

	setFormLayout(form);
	layout->addWidget(rootGroup);
}

void PatternEditor::createStemGroup(QBoxLayout *layout)
{
	std::function<void(StemData *)> f[DSSize] = {
		[&] (StemData *d) {d->density =
			this->dsv[SDensity]->value();},
		[&] (StemData *d) {d->distance =
			this->dsv[SDistance]->value();},
		[&] (StemData *d) {d->length =
			this->dsv[SLength]->value();},
		[&] (StemData *d) {d->radius =
			this->dsv[SRadius]->value();},
		[&] (StemData *d) {d->angleVariation =
			this->dsv[SAngleVariation]->value();},
		[&] (StemData *d) {d->noise =
			this->dsv[SNoise]->value();},
		[&] (StemData *d) {d->radiusThreshold =
			this->dsv[SRadiusThreshold]->value();},
		[&] (StemData *d) {d->fork =
			this->dsv[SFork]->value();},
		[&] (StemData *d) {d->forkAngle =
			this->dsv[SForkAngle]->value();},
		[&] (StemData *d) {d->forkScale =
			this->dsv[SForkScale]->value();},
		[&] (StemData *d) {d->inclineVariation =
			this->dsv[SInclineVariation]->value();},
		[&] (StemData *d) {d->gravity =
			this->dsv[SPull]->value();},
		[&] (StemData *d) {d->radiusVariation =
			this->dsv[SRadiusVariation]->value();},
		[&] (StemData *d) {d->pointDensity =
			this->dsv[SPointDensity]->value();}
	};

	for (int i = 0; i < DSSize; i++) {
		this->dsv[i] = new DoubleSpinBox(this);
		this->dsv[i]->setSingleStep(0.001);
		this->dsv[i]->setDecimals(3);
		connect(this->dsv[i],
			QOverload<double>::of(&DoubleSpinBox::valueChanged),
			std::bind(&PatternEditor::changeField, this, f[i]));
	}

	QGroupBox *stemGroup = createGroup("Stem");
	QFormLayout *form = createForm(stemGroup);
	const float min = std::numeric_limits<float>::lowest();
	const float max = std::numeric_limits<float>::max();
	form->addRow("Stem Density", this->dsv[SDensity]);
	form->addRow("Point Density", this->dsv[SPointDensity]);
	this->dsv[SPointDensity]->setSingleStep(0.01);
	form->addRow("Distance", this->dsv[SDistance]);
	form->addRow("Length", this->dsv[SLength]);
	this->dsv[SLength]->setSingleStep(0.01);
	this->dsv[SLength]->setRange(0.0, std::numeric_limits<float>::max());
	form->addRow("Noise", this->dsv[SNoise]);
	form->addRow("Gravity", this->dsv[SPull]);
	this->dsv[SPull]->setRange(min, max);
	form->addRow("Bifurcation", this->dsv[SFork]);
	form->addRow("Bifurcation Angle", this->dsv[SForkAngle]);
	form->addRow("Bifurcation Radius", this->dsv[SForkScale]);
	form->addRow("Angle Variation", this->dsv[SAngleVariation]);
	form->addRow("Incline Variation", this->dsv[SInclineVariation]);
	form->addRow("Radius Threshold", this->dsv[SRadiusThreshold]);
	form->addRow("Radius Variation", this->dsv[SRadiusVariation]);
	form->addRow("Radius", this->dsv[SRadius]);
	setFormLayout(form);
	layout->addWidget(stemGroup);
}

void PatternEditor::createLeafGroup(QBoxLayout *layout)
{
	std::function<void(StemData *)> fd[DLSize] = {
		[&] (StemData *d) {d->leaf.density =
			this->dlv[LDensity]->value();},
		[&] (StemData *d) {d->leaf.distance =
			this->dlv[LDistance]->value();},
		[&] (StemData *d) {d->leaf.rotation =
			this->dlv[LRotation]->value();},
		[&] (StemData *d) {d->leaf.minUp =
			this->dlv[LMinUp]->value();},
		[&] (StemData *d) {d->leaf.maxUp =
			this->dlv[LMaxUp]->value();},
		[&] (StemData *d) {d->leaf.localUp =
			this->dlv[LLocalUp]->value();},
		[&] (StemData *d) {d->leaf.globalUp =
			this->dlv[LGlobalUp]->value();},
		[&] (StemData *d) {d->leaf.minForward =
			this->dlv[LMinForward]->value();},
		[&] (StemData *d) {d->leaf.maxForward =
			this->dlv[LMaxForward]->value();},
		[&] (StemData *d) {d->leaf.gravity =
			this->dlv[LPull]->value();},
		[&] (StemData *d) {d->leaf.scale.x =
			this->dlv[LScaleX]->value();},
		[&] (StemData *d) {d->leaf.scale.y =
			this->dlv[LScaleY]->value();},
		[&] (StemData *d) {d->leaf.scale.z =
			this->dlv[LScaleZ]->value();}
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

	QGroupBox *leafGroup = createGroup("Leaf");
	QFormLayout *form = createForm(leafGroup);
	form->addRow("Node Density", this->dlv[LDensity]);
	form->addRow("Leaves/Node", this->ilv[LeavesPerNode]);
	form->addRow("Distance", this->dlv[LDistance]);
	form->addRow("Rotation", this->dlv[LRotation]);
	const float min = std::numeric_limits<float>::lowest();
	const float max = std::numeric_limits<float>::max();
	this->dlv[LRotation]->setRange(min, max);
	form->addRow("Min Up", this->dlv[LMinUp]);
	this->dlv[LMinUp]->setRange(min, max);
	form->addRow("Max Up", this->dlv[LMaxUp]);
	this->dlv[LMaxUp]->setRange(min, max);
	form->addRow("Local Up", this->dlv[LLocalUp]);
	this->dlv[LLocalUp]->setRange(min, max);
	form->addRow("Global Up", this->dlv[LGlobalUp]);
	this->dlv[LGlobalUp]->setRange(min, max);
	form->addRow("Min Forward", this->dlv[LMinForward]);
	this->dlv[LMinForward]->setRange(min, max);
	form->addRow("Max Forward", this->dlv[LMaxForward]);
	this->dlv[LMaxForward]->setRange(min, max);
	form->addRow("Gravity", this->dlv[LPull]);
	this->dlv[LPull]->setRange(min, max);
	form->addRow("Scale.X", this->dlv[LScaleX]);
	form->addRow("Scale.Y", this->dlv[LScaleY]);
	form->addRow("Scale.Z", this->dlv[LScaleZ]);
	setFormLayout(form);
	layout->addWidget(leafGroup);
}

StemData PatternEditor::getRootData(StemData data)
{
	data.seed = this->irv[Seed]->value();
	data.maxDepth = this->irv[RMaxDepth]->value();
	data.length = this->drv[RLength]->value();
	data.fork = this->drv[RFork]->value();
	data.forkAngle = this->drv[RForkAngle]->value();
	data.forkScale = this->drv[RForkScale]->value();
	data.radiusThreshold = this->drv[RThreshold]->value();
	data.noise = this->drv[RNoise]->value();
	data.gravity = this->drv[RPull]->value();
	data.pointDensity = this->drv[RPointDensity]->value();
	return data;
}

void PatternEditor::setStemData(StemData data)
{
	this->dsv[SDensity]->setValue(data.density);
	this->dsv[SDistance]->setValue(data.distance);
	this->dsv[SLength]->setValue(data.length);
	this->dsv[SFork]->setValue(data.fork);
	this->dsv[SForkAngle]->setValue(data.forkAngle);
	this->dsv[SForkScale]->setValue(data.forkScale);
	this->dsv[SNoise]->setValue(data.noise);
	this->dsv[SRadius]->setValue(data.radius);
	this->dsv[SAngleVariation]->setValue(data.angleVariation);
	this->dsv[SRadiusThreshold]->setValue(data.radiusThreshold);
	this->dsv[SInclineVariation]->setValue(data.inclineVariation);
	this->dsv[SRadiusVariation]->setValue(data.radiusVariation);
	this->dsv[SPointDensity]->setValue(data.pointDensity);
	this->dsv[SPull]->setValue(data.gravity);
	setLeafData(data.leaf);
}

void PatternEditor::setLeafData(LeafData data)
{
	this->dlv[LDensity]->setValue(data.density);
	this->dlv[LDistance]->setValue(data.distance);
	this->dlv[LRotation]->setValue(data.rotation/pi*180.0f);
	this->dlv[LMinUp]->setValue(data.minUp);
	this->dlv[LMaxUp]->setValue(data.maxUp);
	this->dlv[LLocalUp]->setValue(data.localUp);
	this->dlv[LGlobalUp]->setValue(data.globalUp);
	this->dlv[LMinForward]->setValue(data.minForward);
	this->dlv[LMaxForward]->setValue(data.maxForward);
	this->dlv[LPull]->setValue(data.gravity);
	this->dlv[LScaleX]->setValue(data.scale.x);
	this->dlv[LScaleY]->setValue(data.scale.y);
	this->dlv[LScaleZ]->setValue(data.scale.z);
	this->ilv[LeavesPerNode]->setValue(data.leavesPerNode);
}

void PatternEditor::setEnabled(bool enable)
{
	/* Prevent the scrollbar from jumping to the next focused widget. */
	clearFocusIfDescendant(this);

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
		this->irv[RMaxDepth]->setValue(data.maxDepth);
		this->drv[RLength]->setValue(data.length);
		this->drv[RFork]->setValue(data.fork);
		this->drv[RForkAngle]->setValue(data.forkAngle);
		this->drv[RForkScale]->setValue(data.forkScale);
		this->drv[RNoise]->setValue(data.noise);
		this->drv[RThreshold]->setValue(data.radiusThreshold);
		this->drv[RPull]->setValue(data.gravity);
		this->drv[RPointDensity]->setValue(data.pointDensity);
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
	vlayout->setContentsMargins(0, 0, 0, 0);
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
	this->curveType->addItem("Stem Length");
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

	Spline spline;
	int index = this->curveType->currentIndex();
	Stem *stem = instances.begin()->first;
	ParameterTree tree = stem->getParameterTree();

	if (this->curveNode->currentIndex() > 0) {
		string name = this->curveNode->currentText().toStdString();
		ParameterNode *node = tree.get(name);
		if (index == 0)
			spline = node->getData().densityCurve;
		else if (index == 1)
			spline = node->getData().inclineCurve;
		else if (index == 2)
			spline = node->getData().lengthCurve;
		else
			spline = node->getData().leaf.densityCurve;
		this->curveEditor->setSpline(spline);
	}

	this->curveDegree->blockSignals(true);
	this->curveDegree->setCurrentIndex(spline.getDegree() == 3 ? 1 : 0);
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
			else if (index == 1)
				data.inclineCurve = spline;
			else if (index == 2)
				data.lengthCurve = spline;
			else
				data.leaf.densityCurve = spline;
			node->setData(data);
			stem->setParameterTree(tree);
		}
	}
}

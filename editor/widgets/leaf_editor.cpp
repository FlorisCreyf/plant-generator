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

#include "leaf_editor.h"
#include <iterator>
#include <string>

using pg::Plant;
using pg::Stem;
using pg::Leaf;
using pg::Vec3;
using std::set;
using std::string;
using std::map;

LeafEditor::LeafEditor(
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

QSize LeafEditor::sizeHint() const
{
	return this->leafGroup->sizeHint();
}

void LeafEditor::createInterface()
{
	this->leafGroup = new QGroupBox(tr("Leaf"), this);
	this->leafGroup->setSizePolicy(
		QSizePolicy::Expanding, QSizePolicy::Maximum);

	QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
	layout->setMargin(0);
	layout->setSpacing(0);
	layout->addWidget(this->leafGroup);

	QFormLayout *form = new QFormLayout(this->leafGroup);
	form->setSpacing(2);
	form->setMargin(5);

	this->scaleXLabel = new QLabel(tr("Scale (X)"));
	this->scaleXValue = new QDoubleSpinBox;
	this->scaleXValue->setMinimum(0.01);
	this->scaleXValue->setSingleStep(0.1);
	form->addRow(this->scaleXLabel, this->scaleXValue);

	this->scaleYLabel = new QLabel(tr("Scale (Y)"));
	this->scaleYValue = new QDoubleSpinBox;
	this->scaleYValue->setMinimum(0.01);
	this->scaleYValue->setSingleStep(0.1);
	form->addRow(this->scaleYLabel, this->scaleYValue);

	this->scaleZLabel = new QLabel(tr("Scale (Z)"));
	this->scaleZValue = new QDoubleSpinBox;
	this->scaleZValue->setMinimum(0.01);
	this->scaleZValue->setSingleStep(0.1);
	form->addRow(this->scaleZLabel, this->scaleZValue);

	this->materialLabel = new QLabel(tr("Material"));
	this->materialValue = new QComboBox;
	this->materialValue->addItem(tr(""), QVariant(0));
	form->addRow(this->materialLabel, this->materialValue);

	this->meshLabel = new QLabel(tr("Mesh"));
	this->meshValue = new QComboBox;
	this->meshValue->addItem(tr(""), QVariant(0));
	form->addRow(this->meshLabel, this->meshValue);

	this->customLabel = new QLabel(tr("Custom"));
	this->customValue = new QCheckBox;
	form->addRow(this->customLabel, this->customValue);

	enable(false);
	setValueWidths(form);

	connect(this->customValue, SIGNAL(stateChanged(int)),
		this, SLOT(changeCustom(int)));
	connect(this->scaleXValue, SIGNAL(valueChanged(double)),
		this, SLOT(changeXScale(double)));
	connect(this->scaleXValue, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
	connect(this->scaleYValue, SIGNAL(valueChanged(double)),
		this, SLOT(changeYScale(double)));
	connect(this->scaleYValue, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
	connect(this->scaleZValue, SIGNAL(valueChanged(double)),
		this, SLOT(changeZScale(double)));
	connect(this->scaleZValue, SIGNAL(editingFinished()),
		this, SLOT(finishChanging()));
	connect(this->materialValue, SIGNAL(currentIndexChanged(int)),
		this, SLOT(changeLeafMaterial()));
	connect(this->meshValue, SIGNAL(currentIndexChanged(int)),
		this, SLOT(changeLeafMesh()));
}

void LeafEditor::setFields(map<Stem *, set<size_t>> instances)
{
	if (instances.empty()) {
		enable(false);
		return;
	}

	Stem *stem = instances.rbegin()->first;
	Leaf *leaf = stem->getLeaf(*instances.rbegin()->second.begin());
	blockSignals(true);

	indicateSimilarities(this->customLabel);
	for (auto it = instances.begin(); it != instances.end(); it++) {
		Stem *stem = it->first;
		for (size_t index : it->second) {
			bool a = stem->getLeaf(index)->isCustom();
			bool b = stem->getLeaf(index)->isCustom();
			if (a != b) {
				indicateDifferences(this->customLabel);
				break;
			}
		}
	}
	this->customValue->setCheckState(
		leaf->isCustom() ? Qt::Checked : Qt::Unchecked);

	indicateSimilarities(this->scaleXLabel);
	indicateSimilarities(this->scaleYLabel);
	indicateSimilarities(this->scaleZLabel);
	for (auto it = instances.begin(); it != instances.end(); it++) {
		Stem *stem = it->first;
		for (size_t index : it->second) {
			Vec3 scale = stem->getLeaf(index)->getScale();
			if (scale.x != leaf->getScale().x)
				indicateDifferences(this->scaleXLabel);
			if (scale.y != leaf->getScale().y)
				indicateDifferences(this->scaleYLabel);
			if (scale.z != leaf->getScale().z)
				indicateDifferences(this->scaleZLabel);
		}
	}
	this->scaleXValue->setValue(leaf->getScale().x);
	this->scaleYValue->setValue(leaf->getScale().y);
	this->scaleZValue->setValue(leaf->getScale().z);

	indicateSimilarities(this->materialLabel);
	indicateSimilarities(this->meshLabel);
	for (auto it = instances.begin(); it != instances.end(); it++) {
		Stem *stem = it->first;
		for (size_t index : it->second) {
			Leaf *l = stem->getLeaf(index);
			long material = l->getMaterial();
			long mesh = l->getMesh();
			if (material != leaf->getMaterial())
				indicateDifferences(this->materialLabel);
			if (mesh != leaf->getMesh())
				indicateDifferences(this->meshLabel);
		}
	}

	{
		size_t index = leaf->getMaterial();
		string s = this->shared->getMaterial(index).getName();
		QString qs = QString::fromStdString(s);
		this->materialValue->setCurrentText(qs);
	}

	if (leaf->getMesh() != 0) {
		size_t index = leaf->getMesh();
		Plant *plant = this->editor->getPlant();
		pg::Geometry geom = plant->getLeafMesh(index);
		QString qs = QString::fromStdString(geom.getName());
		this->meshValue->setCurrentText(qs);
	} else
		this->meshValue->setCurrentText(tr(""));

	blockSignals(false);
	enable(true);
}

void LeafEditor::blockSignals(bool block)
{
	this->scaleXValue->blockSignals(block);
	this->scaleYValue->blockSignals(block);
	this->scaleZValue->blockSignals(block);
	this->materialValue->blockSignals(block);
	this->meshValue->blockSignals(block);
	this->customValue->blockSignals(block);
}

void LeafEditor::enable(bool enable)
{
	if (!enable) {
		indicateSimilarities(this->scaleXValue);
		indicateSimilarities(this->scaleYValue);
		indicateSimilarities(this->scaleZValue);
		indicateSimilarities(this->materialValue);
		indicateSimilarities(this->meshValue);
		indicateSimilarities(this->customValue);
	}
	this->scaleXValue->setEnabled(enable);
	this->scaleYValue->setEnabled(enable);
	this->scaleZValue->setEnabled(enable);
	this->materialValue->setEnabled(enable);
	this->meshValue->setEnabled(enable);
	this->customValue->setEnabled(enable);
}

bool LeafEditor::addMaterial(ShaderParams params)
{
	QString name = QString::fromStdString(params.getName());
	if (this->materialValue->findText(name) < 0) {
		qlonglong id = params.getID();
		this->materialValue->addItem(name, QVariant(id));
		return true;
	}
	return false;
}

long LeafEditor::removeMaterial(QString name)
{
	long id = 0;
	int index = this->materialValue->findText(name);
	if (index != 0) {
		id = this->materialValue->itemData(index).toInt();
		this->materialValue->removeItem(index);
	}
	return id;
}

void LeafEditor::renameMaterial(QString before, QString after)
{
	int index = this->materialValue->findText(before);
	this->materialValue->setItemText(index, after);
}

void LeafEditor::addMesh(pg::Geometry geom)
{
	QString name = QString::fromStdString(geom.getName());
	if (this->meshValue->findText(name) < 0) {
		qlonglong id = geom.getID();
		this->meshValue->addItem(name, QVariant(id));
	}
}

void LeafEditor::renameMesh(QString before, QString after)
{
	int index = this->meshValue->findText(before);
	this->meshValue->setItemText(index, after);
}

bool LeafEditor::removeMesh(QString name)
{
	int index = this->meshValue->findText(name);
	if (index != 0) {
		this->meshValue->removeItem(index);
		return true;
	}
	return false;
}

void LeafEditor::changeCustom(int custom)
{
	beginChanging();
	auto instances = this->editor->getSelection()->getLeafInstances();
	for (auto &instance : instances) {
		Stem *stem = instance.first;
		for (size_t index : instance.second)
			stem->getLeaf(index)->setCustom(custom);
	}
	finishChanging();
}

void LeafEditor::changeXScale(double xscale)
{
	beginChanging();
	indicateSimilarities(this->scaleXLabel);
	auto instances = this->editor->getSelection()->getLeafInstances();
	for (auto &instance : instances) {
		Stem *stem = instance.first;
		for (size_t index : instance.second) {
			Vec3 scale = stem->getLeaf(index)->getScale();
			scale.x = xscale;
			stem->getLeaf(index)->setScale(scale);
		}
	}
	this->editor->change();
}

void LeafEditor::changeYScale(double yscale)
{
	beginChanging();
	indicateSimilarities(this->scaleYLabel);
	auto instances = this->editor->getSelection()->getLeafInstances();
	for (auto &instance : instances) {
		Stem *stem = instance.first;
		for (size_t index : instance.second) {
			Vec3 scale = stem->getLeaf(index)->getScale();
			scale.y = yscale;
			stem->getLeaf(index)->setScale(scale);
		}
	}
	this->editor->change();
}

void LeafEditor::changeZScale(double zscale)
{
	beginChanging();
	indicateSimilarities(this->scaleYLabel);
	auto instances = this->editor->getSelection()->getLeafInstances();
	for (auto &instance : instances) {
		Stem *stem = instance.first;
		for (size_t index : instance.second) {
			Vec3 scale = stem->getLeaf(index)->getScale();
			scale.z = zscale;
			stem->getLeaf(index)->setScale(scale);
		}
	}
	this->editor->change();
}

void LeafEditor::changeLeafMaterial()
{
	beginChanging();
	indicateSimilarities(this->materialLabel);
	int id = this->materialValue->currentData().toInt();
	auto instances = this->editor->getSelection()->getLeafInstances();
	for (auto &instance : instances) {
		Stem *stem = instance.first;
		for (size_t index : instance.second)
			stem->getLeaf(index)->setMaterial(id);
	}
	this->editor->change();
	finishChanging();
}

void LeafEditor::changeLeafMesh()
{
	beginChanging();
	indicateSimilarities(this->meshLabel);
	int id = this->meshValue->currentData().toInt();
	auto instances = this->editor->getSelection()->getLeafInstances();
	for (auto &instance : instances) {
		Stem *stem = instance.first;
		for (size_t index : instance.second)
			stem->getLeaf(index)->setMesh(id);
	}
	this->editor->change();
	finishChanging();
}

void LeafEditor::beginChanging()
{
	if (!this->saveStem) {
		this->saveStem = new SaveStem(this->editor->getSelection());
		this->saveStem->execute();
	}
}

void LeafEditor::finishChanging()
{
	if (this->saveStem && !this->saveStem->isSameAsCurrent()) {
		this->saveStem->setNewSelection();
		this->editor->add(this->saveStem);
		/* The history will delete the command. */
		this->saveStem = nullptr;
	}
}

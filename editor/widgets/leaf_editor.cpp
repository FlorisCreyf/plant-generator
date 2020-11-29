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
#include "definitions.h"
#include "form.h"
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
	QWidget(parent), shared(shared), editor(editor), saveStem(nullptr)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	createInterface();
	enable(false);
}

QSize LeafEditor::sizeHint() const
{
	return this->leafGroup->sizeHint();
}

void LeafEditor::createInterface()
{
	this->leafGroup = new QGroupBox("Leaf", this);
	this->leafGroup->setSizePolicy(
		QSizePolicy::Expanding, QSizePolicy::Maximum);

	QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
	layout->setMargin(0);
	layout->setSpacing(0);
	layout->addWidget(this->leafGroup);

	QFormLayout *form = new QFormLayout(this->leafGroup);
	form->setSpacing(UI_FORM_SPACING);
	form->setMargin(UI_FORM_MARGIN);

	this->scaleXLabel = new QLabel("Scale.X");
	this->scaleXValue = new QDoubleSpinBox;
	this->scaleXValue->setMinimum(0.01);
	this->scaleXValue->setSingleStep(0.1);
	form->addRow(this->scaleXLabel, this->scaleXValue);
	connect(this->scaleXValue,
		QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		this, &LeafEditor::changeXScale);
	connect(this->scaleXValue, &QDoubleSpinBox::editingFinished,
		this, &LeafEditor::finishChanging);

	this->scaleYLabel = new QLabel("Scale.Y");
	this->scaleYValue = new QDoubleSpinBox;
	this->scaleYValue->setMinimum(0.01);
	this->scaleYValue->setSingleStep(0.1);
	form->addRow(this->scaleYLabel, this->scaleYValue);
	connect(this->scaleYValue,
		QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		this, &LeafEditor::changeYScale);
	connect(this->scaleYValue, &QDoubleSpinBox::editingFinished,
		this, &LeafEditor::finishChanging);

	this->scaleZLabel = new QLabel("Scale.Z");
	this->scaleZValue = new QDoubleSpinBox;
	this->scaleZValue->setMinimum(0.01);
	this->scaleZValue->setSingleStep(0.1);
	form->addRow(this->scaleZLabel, this->scaleZValue);
	connect(this->scaleZValue,
		QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		this, &LeafEditor::changeZScale);
	connect(this->scaleZValue, &QDoubleSpinBox::editingFinished,
		this, &LeafEditor::finishChanging);

	this->materialLabel = new QLabel("Material");
	this->materialValue = new QComboBox;
	form->addRow(this->materialLabel, this->materialValue);
	connect(this->materialValue,
		QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, &LeafEditor::changeLeafMaterial);

	this->meshLabel = new QLabel("Mesh");
	this->meshValue = new QComboBox;
	form->addRow(this->meshLabel, this->meshValue);
	connect(this->meshValue,
		QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, &LeafEditor::changeLeafMesh);

	this->customLabel = new QLabel("Manual");
	this->customValue = new QCheckBox;
	form->addRow(this->customLabel, this->customValue);
	connect(this->customValue, &QCheckBox::stateChanged,
		this, &LeafEditor::changeCustom);

	enable(false);
	setValueWidths(form);
}

void LeafEditor::setFields(map<Stem *, set<size_t>> instances)
{
	enable(false);
	if (instances.empty())
		return;

	Stem *stem = instances.rbegin()->first;
	Leaf *leaf = stem->getLeaf(*instances.rbegin()->second.begin());
	blockSignals(true);

	Leaf *leaf1 = nullptr;
	for (auto it = instances.begin(); it != instances.end(); it++) {
		Stem *stem = it->first;
		for (size_t index : it->second) {
			Leaf *leaf2 = stem->getLeaf(index);
			if (!leaf1) {
				leaf1 = leaf2;
				continue;
			}

			if (leaf1->isCustom() != leaf2->isCustom())
				indicateDifferences(this->customLabel);
			if (leaf1->getScale().x != leaf2->getScale().x)
				indicateDifferences(this->scaleXLabel);
			if (leaf1->getScale().y != leaf2->getScale().y)
				indicateDifferences(this->scaleYLabel);
			if (leaf1->getScale().z != leaf2->getScale().z)
				indicateDifferences(this->scaleZLabel);
			if (leaf1->getMaterial() != leaf2->getMaterial())
				indicateDifferences(this->materialLabel);
			if (leaf1->getMesh() != leaf2->getMesh())
				indicateDifferences(this->meshLabel);
		}
	}

	this->customValue->setCheckState(
		leaf->isCustom() ? Qt::Checked : Qt::Unchecked);
	this->scaleXValue->setValue(leaf->getScale().x);
	this->scaleYValue->setValue(leaf->getScale().y);
	this->scaleZValue->setValue(leaf->getScale().z);
	{
		size_t index = leaf->getMaterial();
		string s = this->shared->getMaterial(index).getName();
		QString qs = QString::fromStdString(s);
		this->materialValue->setCurrentText(qs);
	}
	{
		size_t index = leaf->getMesh();
		Plant *plant = this->editor->getPlant();
		pg::Geometry geom = plant->getLeafMesh(index);
		QString qs = QString::fromStdString(geom.getName());
		this->meshValue->setCurrentText(qs);
	}

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
		indicateSimilarities(this->scaleXLabel);
		indicateSimilarities(this->scaleYLabel);
		indicateSimilarities(this->scaleZLabel);
		indicateSimilarities(this->materialLabel);
		indicateSimilarities(this->meshLabel);
		indicateSimilarities(this->customLabel);
	}
	this->scaleXValue->setEnabled(enable);
	this->scaleYValue->setEnabled(enable);
	this->scaleZValue->setEnabled(enable);
	this->materialValue->setEnabled(enable);
	this->meshValue->setEnabled(enable);
	this->customValue->setEnabled(enable);
}

void LeafEditor::clearOptions()
{
	blockSignals(true);
	this->materialValue->clear();
	this->meshValue->clear();
	blockSignals(false);
}

bool LeafEditor::addMaterial(ShaderParams params)
{
	QString name = QString::fromStdString(params.getName());
	if (this->materialValue->findText(name) < 0) {
		this->materialValue->blockSignals(true);
		this->materialValue->addItem(name);
		this->materialValue->blockSignals(false);
		return true;
	}
	return false;
}

void LeafEditor::removeMaterial(unsigned index)
{
	this->materialValue->blockSignals(true);
	unsigned currentIndex = this->materialValue->currentIndex();
	if (index == currentIndex)
		this->materialValue->setCurrentIndex(0);
	this->materialValue->removeItem(index);
	this->materialValue->blockSignals(false);
}

void LeafEditor::updateMaterials()
{
	unsigned size = this->shared->getMaterialCount();
	for (unsigned i = 0; i < size; i++) {
		ShaderParams params = this->shared->getMaterial(i);
		QString name = QString::fromStdString(params.getName());
		this->materialValue->setItemText(i, name);
	}
}

void LeafEditor::addMesh(pg::Geometry geom)
{
	QString name = QString::fromStdString(geom.getName());
	if (this->meshValue->findText(name) < 0)
		this->meshValue->addItem(name);
}

void LeafEditor::updateMesh(pg::Geometry geom, unsigned index)
{
	QString name = QString::fromStdString(geom.getName());
	this->meshValue->setItemText(index, name);
}

void LeafEditor::removeMesh(unsigned index)
{
	this->meshValue->blockSignals(true);
	unsigned currentIndex = this->meshValue->currentIndex();
	if (index == currentIndex)
		this->meshValue->setCurrentIndex(0);
	this->meshValue->removeItem(index);
	this->meshValue->blockSignals(false);
}

void LeafEditor::changeCustom(int custom)
{
	beginChanging(this->customLabel);
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
	beginChanging(this->scaleXLabel);
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
	beginChanging(this->scaleYLabel);
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
	beginChanging(this->scaleZLabel);
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
	beginChanging(this->materialLabel);
	unsigned index = this->materialValue->currentIndex();
	auto instances = this->editor->getSelection()->getLeafInstances();
	for (auto &instance : instances) {
		Stem *stem = instance.first;
		for (size_t leafIndex : instance.second)
			stem->getLeaf(leafIndex)->setMaterial(index);
	}
	this->editor->change();
	finishChanging();
}

void LeafEditor::changeLeafMesh()
{
	beginChanging(this->meshLabel);
	unsigned index = this->meshValue->currentIndex();
	auto instances = this->editor->getSelection()->getLeafInstances();
	for (auto &instance : instances) {
		Stem *stem = instance.first;
		for (size_t leafIndex : instance.second)
			stem->getLeaf(leafIndex)->setMesh(index);
	}
	this->editor->change();
	finishChanging();
}

void LeafEditor::beginChanging(QLabel *label)
{
	indicateSimilarities(label);
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
		this->saveStem = nullptr;
	}
}

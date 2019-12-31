/* Plant Genererator
 * Copyright (C) 2016-2018  Floris Creyf
 *
 * Plant Genererator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Plant Genererator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "property_box.h"
#include <iterator>

using pg::Spline;

using std::next;
using std::prev;
using std::string;
using std::map;
using std::set;

PropertyBox::PropertyBox(SharedResources *shared, Editor *editor,
	QWidget *parent) : QWidget(parent)
{
	this->shared = shared;
	curveEditor = nullptr;
	selectedCurve = nullptr;
	changing = false;

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setSizeConstraint(QLayout::SetMinimumSize);
	layout->setSpacing(0);
	layout->setMargin(0);
	createStemBox(layout);
	createCapBox(layout);
	createLeafBox(layout);
	enableStem(false);
	leafMaterialV->setEnabled(false);
	layout->addStretch(1);

	this->editor = editor;
	connect(editor, SIGNAL(selectionChanged()), this, SLOT(fill()));
}

void PropertyBox::setValueWidths(QFormLayout *layout)
{
	layout->setFormAlignment(Qt::AlignRight | Qt::AlignTop);
	layout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
	layout->setLabelAlignment(Qt::AlignRight);
	for(int i = 0; i < layout->rowCount(); i++) {
		QLayoutItem *item = layout->itemAt(i, QFormLayout::FieldRole);
		if (item && item->widget())
			item->widget()->setFixedWidth(250);
	}
}

void PropertyBox::createStemBox(QVBoxLayout *layout)
{
	stemG = new QGroupBox(tr("Stem"));
	QFormLayout *form = new QFormLayout(stemG);
	form->setSpacing(2);
	form->setMargin(10);

	{
		QWidget *sizeWidget = new QWidget();
		QHBoxLayout *line = new QHBoxLayout();
		sizeWidget->setLayout(line);
		radiusL = new QLabel(tr("Radius"));
		radiusV = new QDoubleSpinBox;
		radiusB = new CurveButton("Radius", shared, this);
		radiusB->setFixedWidth(22);
		radiusB->setFixedHeight(22);
		radiusV->setSingleStep(0.001);
		radiusV->setDecimals(3);
		line->addWidget(radiusV);
		line->addWidget(radiusB);
		line->setSpacing(0);
		line->setMargin(0);
		form->addRow(radiusL, sizeWidget);
	}

	minRadiusL = new QLabel(tr("Min Radius"));
	minRadiusV = new QDoubleSpinBox;
	minRadiusV->setSingleStep(0.001);
	minRadiusV->setDecimals(3);
	form->addRow(minRadiusL, minRadiusV);

	divisionL = new QLabel(tr("Divisions"));
	divisionV = new QSpinBox;
	divisionV->setMinimum(1);
	form->addRow(divisionL, divisionV);

	resolutionL = new QLabel(tr("Resolution"));
	resolutionV = new QSpinBox;
	resolutionV->setMinimum(5);
	form->addRow(resolutionL, resolutionV);

	degreeL = new QLabel(tr("Degree"));
	degreeV = new QComboBox;
	degreeV->addItem(QString("Linear"));
	degreeV->addItem(QString("Cubic"));
	form->addRow(degreeL, degreeV);

	stemMaterialL = new QLabel(tr("Material"));
	stemMaterialV = new QComboBox;
	stemMaterialV->addItem(tr(""), QVariant(0));
	form->addRow(stemMaterialL, stemMaterialV);

	setValueWidths(form);
	stemG->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	layout->addWidget(stemG);

	connect(degreeV, SIGNAL(currentIndexChanged(int)), this,
		SLOT(changePathDegree(int)));
	connect(resolutionV, SIGNAL(editingFinished()), this,
		SLOT(finishChanging()));
	connect(resolutionV, SIGNAL(valueChanged(int)), this,
		SLOT(changeResolution(int)));
	connect(divisionV, SIGNAL(editingFinished()), this,
		SLOT(finishChanging()));
	connect(divisionV, SIGNAL(valueChanged(int)), this,
		SLOT(changeDivisions(int)));
	connect(radiusV, SIGNAL(editingFinished()), this,
		SLOT(finishChanging()));
	connect(radiusV, SIGNAL(valueChanged(double)), this,
		SLOT(changeRadius(double)));
	connect(minRadiusV, SIGNAL(editingFinished()), this,
		SLOT(finishChanging()));
	connect(minRadiusV, SIGNAL(valueChanged(double)), this,
		SLOT(changeMinRadius(double)));
	connect(radiusB, SIGNAL(selected(CurveButton *)), this,
		SLOT(toggleCurve(CurveButton *)));
	connect(stemMaterialV, SIGNAL(currentIndexChanged(int)), this,
		SLOT(changeStemMaterial()));
}

void PropertyBox::createCapBox(QVBoxLayout *layout)
{
	capG = new QGroupBox(tr("Cap"));
	QFormLayout *form = new QFormLayout(capG);
	form->setSpacing(2);
	form->setMargin(10);

	capMaterialL = new QLabel(tr("Material"));
	capMaterialV = new QComboBox;
	capMaterialV->addItem(tr(""), QVariant(0));
	form->addRow(capMaterialL, capMaterialV);

	setValueWidths(form);
	capG->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	layout->addWidget(capG);

	connect(capMaterialV, SIGNAL(currentIndexChanged(int)), this,
		SLOT(changeCapMaterial()));
}

void PropertyBox::createLeafBox(QVBoxLayout *layout)
{
	leafG = new QGroupBox(tr("Leaf"));
	QFormLayout *form = new QFormLayout(leafG);
	form->setSpacing(2);
	form->setMargin(10);

	leafScaleXL = new QLabel(tr("X Scale"));
	leafScaleXV = new QDoubleSpinBox;
	leafScaleXV->setMinimum(0.01);
	leafScaleXV->setSingleStep(0.1);
	form->addRow(leafScaleXL, leafScaleXV);

	leafScaleYL = new QLabel(tr("Y Scale"));
	leafScaleYV = new QDoubleSpinBox;
	leafScaleYV->setMinimum(0.01);
	leafScaleYV->setSingleStep(0.1);
	form->addRow(leafScaleYL, leafScaleYV);

	leafScaleZL = new QLabel(tr("Z Scale"));
	leafScaleZV = new QDoubleSpinBox;
	leafScaleZV->setMinimum(0.01);
	leafScaleZV->setSingleStep(0.1);
	form->addRow(leafScaleZL, leafScaleZV);

	leafMaterialL = new QLabel(tr("Material"));
	leafMaterialV = new QComboBox;
	leafMaterialV->addItem(tr(""), QVariant(0));
	form->addRow(leafMaterialL, leafMaterialV);

	leafMeshL = new QLabel(tr("Mesh"));
	leafMeshV = new QComboBox;
	leafMeshV->addItem(tr(""), QVariant(0));
	form->addRow(leafMeshL, leafMeshV);

	enableLeaf(false);
	setValueWidths(form);
	leafG->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	layout->addWidget(leafG);

	connect(leafScaleXV, SIGNAL(valueChanged(double)), this,
		SLOT(changeXScale(double)));
	connect(leafScaleXV, SIGNAL(editingFinished()), this,
		SLOT(finishChanging()));
	connect(leafScaleYV, SIGNAL(valueChanged(double)), this,
		SLOT(changeYScale(double)));
	connect(leafScaleYV, SIGNAL(editingFinished()), this,
		SLOT(finishChanging()));
	connect(leafScaleZV, SIGNAL(valueChanged(double)), this,
		SLOT(changeZScale(double)));
	connect(leafScaleZV, SIGNAL(editingFinished()), this,
		SLOT(finishChanging()));
	connect(leafMaterialV, SIGNAL(currentIndexChanged(int)), this,
		SLOT(changeLeafMaterial()));
	connect(leafMeshV, SIGNAL(currentIndexChanged(int)), this,
		SLOT(changeLeafMesh()));
}

void PropertyBox::fill()
{
	auto leafInstances = editor->getSelection()->getLeafInstances();
	auto stemInstances = editor->getSelection()->getStemInstances();

	if (stemInstances.empty()) {
		enableStem(false);
		curveEditor->setEnabled(false);
	} else
		setStemFields(stemInstances);

	if (leafInstances.empty())
		enableLeaf(false);
	else
		setLeafFields(leafInstances);
}

void PropertyBox::setStemFields(map<pg::Stem *, PointSelection> instances)
{
	pg::Stem *stem = instances.rbegin()->first;
	auto nextIt = next(instances.begin());

	indicateSimilarities(resolutionL);
	for (auto it = nextIt; it != instances.end(); ++it) {
		pg::Stem *a = prev(it)->first;
		pg::Stem *b = it->first;
		if (a->getResolution() != b->getResolution()) {
			indicateDifferences(resolutionL);
			break;
		}
	}

	resolutionV->blockSignals(true);
	resolutionV->setValue(stem->getResolution());
	resolutionV->blockSignals(false);

	indicateSimilarities(divisionL);
	for (auto it = nextIt; it != instances.end(); ++it) {
		pg::Path a = prev(it)->first->getPath();
		pg::Path b = it->first->getPath();
		if (a.getResolution() != b.getResolution()) {
			indicateDifferences(divisionL);
			break;
		}
	}

	divisionV->blockSignals(true);
	divisionV->setValue(stem->getPath().getResolution());
	divisionV->blockSignals(false);

	indicateSimilarities(radiusL);
	for (auto it = nextIt; it != instances.end(); ++it) {
		pg::Path a = prev(it)->first->getPath();
		pg::Path b = it->first->getPath();
		if (a.getMaxRadius() != b.getMaxRadius()) {
			indicateDifferences(radiusL);
			break;
		}
	}

	radiusV->blockSignals(true);
	radiusV->setValue(stem->getPath().getMaxRadius());
	radiusV->blockSignals(false);
	/* TODO make curve bold if radius curves are different. */
	radiusB->blockSignals(true);
	radiusB->setCurve(stem->getPath().getRadius());
	radiusB->blockSignals(false);

	indicateSimilarities(minRadiusL);
	for (auto it = nextIt; it != instances.end(); ++it) {
		pg::Path a = prev(it)->first->getPath();
		pg::Path b = it->first->getPath();
		if (a.getMinRadius() != b.getMinRadius()) {
			indicateDifferences(minRadiusL);
			break;
		}
	}

	minRadiusV->blockSignals(true);
	minRadiusV->setValue(stem->getPath().getMinRadius());
	minRadiusV->blockSignals(false);

	indicateSimilarities(degreeL);
	for (auto it = nextIt; it != instances.end(); ++it) {
		Spline a = prev(it)->first->getPath().getSpline();
		Spline b = it->first->getPath().getSpline();
		if (a.getDegree() != b.getDegree()) {
			indicateDifferences(degreeL);
			break;
		}
	}

	degreeV->blockSignals(true);
	switch (stem->getPath().getSpline().getDegree()) {
	case 1:
		degreeV->setCurrentIndex(0);
		break;
	case 3:
		degreeV->setCurrentIndex(1);
		break;
	}
	degreeV->blockSignals(false);

	indicateSimilarities(stemMaterialL);
	for (auto it = nextIt; it != instances.end(); ++it) {
		int a = prev(it)->first->getMaterial(pg::Stem::Outer);
		int b = it->first->getMaterial(pg::Stem::Outer);
		if (a != b) {
			indicateDifferences(stemMaterialL);
			break;
		}
	}

	stemMaterialV->blockSignals(true);
	{
		int id = stem->getMaterial(pg::Stem::Outer);
		string s = shared->getMaterial(id).getName();
		QString qs = QString::fromStdString(s);
		stemMaterialV->setCurrentText(qs);
	}
	stemMaterialV->blockSignals(false);

	indicateSimilarities(capMaterialL);
	for (auto it = nextIt; it != instances.end(); ++it) {
		int a = prev(it)->first->getMaterial(pg::Stem::Inner);
		int b = it->first->getMaterial(pg::Stem::Inner);
		if (a != b) {
			indicateDifferences(capMaterialL);
			break;
		}
	}

	capMaterialV->blockSignals(true);
	{
		int id = stem->getMaterial(pg::Stem::Inner);
		string s = shared->getMaterial(id).getName();
		QString qs = QString::fromStdString(s);
		capMaterialV->setCurrentText(qs);
	}
	capMaterialV->blockSignals(false);

	if (selectedCurve)
		selectedCurve->select();

	enableStem(true);
	curveEditor->setEnabled(true);
	stemG->blockSignals(false);
}

void PropertyBox::setLeafFields(map<pg::Stem *, set<long>> instances)
{
	pg::Stem *stem = instances.rbegin()->first;
	unsigned index = *instances.rbegin()->second.begin();
	pg::Leaf *leaf = stem->getLeaf(index);

	indicateSimilarities(leafScaleXL);
	indicateSimilarities(leafScaleYL);
	indicateSimilarities(leafScaleZL);
	for (auto it = instances.begin(); it != instances.end(); it++) {
		pg::Stem *stem = it->first;
		for (auto &id : it->second) {
			pg::Vec3 scale = stem->getLeaf(id)->getScale();
			if (scale.x != leaf->getScale().x)
				indicateDifferences(leafScaleXL);
			if (scale.y != leaf->getScale().y)
				indicateDifferences(leafScaleYL);
			if (scale.z != leaf->getScale().z)
				indicateDifferences(leafScaleZL);
		}
	}

	leafScaleXV->blockSignals(true);
	leafScaleXV->setValue(leaf->getScale().x);
	leafScaleXV->blockSignals(false);

	leafScaleYV->blockSignals(true);
	leafScaleYV->setValue(leaf->getScale().y);
	leafScaleYV->blockSignals(false);

	leafScaleZV->blockSignals(true);
	leafScaleZV->setValue(leaf->getScale().z);
	leafScaleZV->blockSignals(false);

	indicateSimilarities(leafMaterialL);
	indicateSimilarities(leafMeshL);
	for (auto it = instances.begin(); it != instances.end(); it++) {
		pg::Stem *stem = it->first;
		for (auto &id : it->second) {
			pg::Leaf *l = stem->getLeaf(id);
			long material = l->getMaterial();
			long mesh = l->getMesh();
			if (material != leaf->getMaterial())
				indicateDifferences(leafMaterialL);
			if (mesh != leaf->getMesh())
				indicateDifferences(leafMeshL);
		}
	}

	leafMaterialV->blockSignals(true);
	{
		int id = leaf->getMaterial();
		string s = shared->getMaterial(id).getName();
		QString qs = QString::fromStdString(s);
		leafMaterialV->setCurrentText(qs);
	}
	leafMaterialV->blockSignals(false);

	leafMeshV->blockSignals(true);
	if (leaf->getMesh() != 0) {
		int id = leaf->getMesh();
		pg::Plant *plant = editor->getPlant();
		pg::Geometry geom = plant->getLeafMesh(id);
		QString qs = QString::fromStdString(geom.getName());
		leafMeshV->setCurrentText(qs);
	} else {
		leafMeshV->setCurrentText(tr(""));
	}
	leafMeshV->blockSignals(false);

	enableLeaf(true);
}

void PropertyBox::addMaterial(ShaderParams params)
{
	QString name;
	name = QString::fromStdString(params.getName());
	if (stemMaterialV->findText(name) < 0) {
		stemMaterialV->addItem(
			name, QVariant((qlonglong)params.getID()));
		capMaterialV->addItem(
			name, QVariant((qlonglong)params.getID()));
		leafMaterialV->addItem(
			name, QVariant((qlonglong)params.getID()));
		editor->getPlant()->addMaterial(params.getMaterial());
	}
	editor->change();
}

void PropertyBox::removeMaterial(QString name)
{
	int index = stemMaterialV->findText(name);
	if (index != 0) {
		long id = stemMaterialV->itemData(index).toInt();
		editor->getPlant()->removeMaterial(id);
		stemMaterialV->removeItem(index);
		index = capMaterialV->findText(name);
		capMaterialV->removeItem(index);
		index = leafMaterialV->findText(name);
		leafMaterialV->removeItem(index);
		editor->change();
	}
}

void PropertyBox::renameMaterial(QString before, QString after)
{
	int index;
	index = stemMaterialV->findText(before);
	stemMaterialV->setItemText(index, after);
	index = capMaterialV->findText(before);
	capMaterialV->setItemText(index, after);
	index = leafMaterialV->findText(before);
	leafMaterialV->setItemText(index, after);
}

void PropertyBox::addMesh(pg::Geometry geom)
{
	QString name;
	name = QString::fromStdString(geom.getName());
	if (leafMeshV->findText(name) < 0)
		leafMeshV->addItem(name, QVariant((qlonglong)geom.getID()));
	editor->change();
}

void PropertyBox::renameMesh(QString before, QString after)
{
	int index = leafMeshV->findText(before);
	leafMeshV->setItemText(index, after);
}

void PropertyBox::removeMesh(QString name)
{
	int index = leafMeshV->findText(name);
	if (index != 0) {
		leafMeshV->removeItem(index);
		editor->change();
	}
}

void PropertyBox::changePathDegree(int i)
{
	beginChanging();
	indicateSimilarities(degreeL);
	int degree = i == 1 ? 3 : 1;
	auto instances = editor->getSelection()->getStemInstances();
	for (auto &instance : instances) {
		pg::Path path = instance.first->getPath();
		Spline spline = path.getSpline();
		if (spline.getDegree() != degree) {
			spline.adjust(degree);
			editor->getSelection()->clearPoints();
		}
		path.setSpline(spline);
		instance.first->setPath(path);
	}
	editor->getSelection()->clearPoints();
	editor->change();
	finishChanging();
}

void PropertyBox::changeResolution(int i)
{
	beginChanging();
	indicateSimilarities(resolutionL);
	auto instances = editor->getSelection()->getStemInstances();
	for (auto &instance : instances)
		instance.first->setResolution(i);
	editor->change();
}

void PropertyBox::changeDivisions(int i)
{
	beginChanging();
	indicateSimilarities(divisionL);
	auto instances = editor->getSelection()->getStemInstances();
	for (auto &instance : instances) {
		pg::Path path = instance.first->getPath();
		path.setResolution(i);
		instance.first->setPath(path);
	}
	editor->change();
}

void PropertyBox::changeRadius(double d)
{
	beginChanging();
	indicateSimilarities(radiusL);
	auto instances = editor->getSelection()->getStemInstances();
	for (auto &instance : instances) {
		pg::Path path = instance.first->getPath();
		path.setMaxRadius(d);
		instance.first->setPath(path);
	}
	editor->change();
}

void PropertyBox::changeMinRadius(double d)
{
	beginChanging();
	indicateSimilarities(minRadiusL);
	auto instances = editor->getSelection()->getStemInstances();
	for (auto &instance : instances) {
		pg::Path path = instance.first->getPath();
		path.setMinRadius(d);
		instance.first->setPath(path);
	}
	editor->change();
}

void PropertyBox::changeRadiusCurve(pg::Spline &spline)
{
	beginChanging();
	auto instances = editor->getSelection()->getStemInstances();
	for (auto &instance : instances) {
		pg::Path vp = instance.first->getPath();
		vp.setRadius(spline);
		instance.first->setPath(vp);
	}
	editor->change();
}

void PropertyBox::changeStemMaterial()
{
	beginChanging();
	indicateSimilarities(stemMaterialL);
	int id = stemMaterialV->currentData().toInt();
	auto instances = editor->getSelection()->getStemInstances();
	for (auto &instance : instances)
		instance.first->setMaterial(pg::Stem::Outer, id);
	editor->change();
	finishChanging();
}

void PropertyBox::changeCapMaterial()
{
	beginChanging();
	indicateSimilarities(capMaterialL);
	int id = capMaterialV->currentData().toInt();
	auto instances = editor->getSelection()->getStemInstances();
	for (auto &instance : instances)
		instance.first->setMaterial(pg::Stem::Inner, id);
	editor->change();
	finishChanging();
}

void PropertyBox::changeXScale(double d)
{
	beginChanging();
	indicateSimilarities(leafScaleXL);
	auto instances = editor->getSelection()->getLeafInstances();
	for (auto &instance : instances) {
		pg::Stem *stem = instance.first;
		for (auto &leaf : instance.second) {
			pg::Vec3 scale = stem->getLeaf(leaf)->getScale();
			scale.x = d;
			stem->getLeaf(leaf)->setScale(scale);
		}
	}
	editor->change();
}

void PropertyBox::changeYScale(double d)
{
	beginChanging();
	indicateSimilarities(leafScaleYL);
	auto instances = editor->getSelection()->getLeafInstances();
	for (auto &instance : instances) {
		pg::Stem *stem = instance.first;
		for (auto &leaf : instance.second) {
			pg::Vec3 scale = stem->getLeaf(leaf)->getScale();
			scale.y = d;
			stem->getLeaf(leaf)->setScale(scale);
		}
	}
	editor->change();
}

void PropertyBox::changeZScale(double d)
{
	beginChanging();
	indicateSimilarities(leafScaleYL);
	auto instances = editor->getSelection()->getLeafInstances();
	for (auto &instance : instances) {
		pg::Stem *stem = instance.first;
		for (auto &leaf : instance.second) {
			pg::Vec3 scale = stem->getLeaf(leaf)->getScale();
			scale.z = d;
			stem->getLeaf(leaf)->setScale(scale);
		}
	}
	editor->change();
}

void PropertyBox::changeLeafMaterial()
{
	beginChanging();
	indicateSimilarities(leafMaterialL);
	int id = leafMaterialV->currentData().toInt();
	auto instances = editor->getSelection()->getLeafInstances();
	for (auto &instance : instances) {
		pg::Stem *stem = instance.first;
		for (auto &leaf : instance.second)
			stem->getLeaf(leaf)->setMaterial(id);
	}
	editor->change();
	finishChanging();
}

void PropertyBox::changeLeafMesh()
{
	beginChanging();
	indicateSimilarities(leafMeshL);
	int id = leafMeshV->currentData().toInt();
	auto instances = editor->getSelection()->getLeafInstances();
	for (auto &instance : instances) {
		pg::Stem *stem = instance.first;
		for (auto &leaf : instance.second)
			stem->getLeaf(leaf)->setMesh(id);
	}
	editor->change();
	finishChanging();
}

void PropertyBox::beginChanging()
{
	if (!changing) {
		saveStem = new SaveStem(editor->getSelection());
		saveStem->execute();
	}
	changing = true;
}

void PropertyBox::finishChanging()
{
	if (changing && !saveStem->isSameAsCurrent()) {
		saveStem->setNewSelection();
		editor->add(*saveStem);
	}
	changing = false;
}

void PropertyBox::enableStem(bool enable)
{
	if (!enable) {
		indicateSimilarities(radiusL);
		indicateSimilarities(minRadiusL);
		indicateSimilarities(resolutionL);
		indicateSimilarities(divisionL);
		indicateSimilarities(degreeL);
		indicateSimilarities(stemMaterialL);
		indicateSimilarities(capMaterialL);
	}

	radiusV->setEnabled(enable);
	radiusB->setEnabled(enable);
	minRadiusV->setEnabled(enable);
	resolutionV->setEnabled(enable);
	divisionV->setEnabled(enable);
	degreeV->setEnabled(enable);
	stemMaterialV->setEnabled(enable);
	capMaterialV->setEnabled(enable);
}

void PropertyBox::enableLeaf(bool enable)
{
	if (!enable) {
		indicateSimilarities(leafScaleXV);
		indicateSimilarities(leafScaleYV);
		indicateSimilarities(leafScaleZV);
		indicateSimilarities(leafMaterialV);
		indicateSimilarities(leafMeshV);
	}

	leafScaleXV->setEnabled(enable);
	leafScaleYV->setEnabled(enable);
	leafScaleZV->setEnabled(enable);
	leafMaterialV->setEnabled(enable);
	leafMeshV->setEnabled(enable);
}

void PropertyBox::indicateDifferences(QWidget *widget)
{
	widget->setStyleSheet("font-weight:bold;");
}

void PropertyBox::indicateSimilarities(QWidget *widget)
{
	widget->setStyleSheet("");
}

void PropertyBox::bind(CurveEditor *curveEditor)
{
	this->curveEditor = curveEditor;
	connect(curveEditor, SIGNAL(curveChanged(pg::Spline, QString)), this,
		SLOT(setCurve(pg::Spline, QString)));
	connect(curveEditor, SIGNAL(editingFinished()), this,
		SLOT(finishChanging()));
}

QSize PropertyBox::sizeHint() const
{
	return QSize(350, 200);
}

void PropertyBox::setCurve(pg::Spline spline, QString name)
{
	if (name == "Radius") {
		radiusB->setCurve(spline);
		changeRadiusCurve(spline);
	}
}

void PropertyBox::toggleCurve(CurveButton *button)
{
	selectedCurve = button;
	auto instances = editor->getSelection()->getStemInstances();
	pg::Stem *stem = nullptr;
	if (!instances.empty()) {
		stem = instances.rbegin()->first;
		QString name = button->getName();
		curveEditor->setCurve(stem->getPath().getRadius(), name);
	}
}

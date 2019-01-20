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

using pg::VolumetricPath;
using pg::Spline;
using std::next;
using std::prev;
using std::string;

PropertyBox::PropertyBox(SharedResources *shared, QWidget *parent) :
	QWidget(parent), saveStem(nullptr)
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
	enable(false);
	leafMaterialV->setEnabled(false);
	layout->addStretch(1);
}

/**
 * Create a container that stores properties that only affect the current stem.
 */
void PropertyBox::createStemBox(QVBoxLayout *layout)
{
	stemG = new QGroupBox(tr("Stem"));
	QFormLayout *form = new QFormLayout(stemG);
	form->setSpacing(3);
	form->setMargin(10);

	{
		QHBoxLayout *line = new QHBoxLayout();
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
		form->addRow(radiusL, line);
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
	form->setSpacing(3);
	form->setMargin(10);

	capMaterialL = new QLabel(tr("Material"));
	capMaterialV = new QComboBox;
	capMaterialV->addItem(tr(""), QVariant(0));
	form->addRow(capMaterialL, capMaterialV);

	capG->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	layout->addWidget(capG);

	connect(capMaterialV, SIGNAL(currentIndexChanged(int)), this,
		SLOT(changeCapMaterial()));
}

void PropertyBox::createLeafBox(QVBoxLayout *layout)
{
	leafG = new QGroupBox(tr("Leaf"));
	QFormLayout *form = new QFormLayout(leafG);
	form->setSpacing(3);
	form->setMargin(10);

	leafMaterialL = new QLabel(tr("Material"));
	leafMaterialV = new QComboBox;
	leafMaterialV->addItem(tr(""), QVariant(0));
	form->addRow(leafMaterialL, leafMaterialV);

	leafG->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	layout->addWidget(leafG);
}

/**
 * Changes the values in each container to reflect the new selection. If
 * multiple stems are selected, then show the latest value and set the
 * property name bold.
 */
void PropertyBox::fill()
{
	auto instances = editor->getSelection()->getInstances();

	if (instances.empty()) {
		enable(false);
		curveEditor->setEnabled(false);
	} else {
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
			VolumetricPath a = prev(it)->first->getPath();
			VolumetricPath b = it->first->getPath();
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
			VolumetricPath a = prev(it)->first->getPath();
			VolumetricPath b = it->first->getPath();
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
			VolumetricPath a = prev(it)->first->getPath();
			VolumetricPath b = it->first->getPath();
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

		enable(true);
		curveEditor->setEnabled(true);
		stemG->blockSignals(false);
	}
}

void PropertyBox::addMaterial(ShaderParams params)
{
	QString name;
	name = QString::fromStdString(params.getName());
	if (stemMaterialV->findText(name) < 0) {
		stemMaterialV->addItem(name, QVariant(params.getId()));
		capMaterialV->addItem(name, QVariant(params.getId()));
		leafMaterialV->addItem(name, QVariant(params.getId()));
		editor->getPlant()->addMaterial(params.getMaterial());
	}
	editor->change();
}

void PropertyBox::removeMaterial(QString name)
{
	int index;
	int id;
	index = stemMaterialV->findText(name);
	if (index != 0) {
		id = stemMaterialV->itemData(index).toInt();
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

void PropertyBox::changePathDegree(int i)
{
	beginChanging();
	indicateSimilarities(degreeL);
	int degree = i == 1 ? 3 : 1;
	auto instances = editor->getSelection()->getInstances();
	for (auto &instance : instances) {
		VolumetricPath path = instance.first->getPath();
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
	auto instances = editor->getSelection()->getInstances();
	for (auto &instance : instances)
		instance.first->setResolution(i);
	editor->change();
}

void PropertyBox::changeDivisions(int i)
{
	beginChanging();
	indicateSimilarities(divisionL);
	auto instances = editor->getSelection()->getInstances();
	for (auto &instance : instances) {
		VolumetricPath vpath = instance.first->getPath();
		vpath.setResolution(i);
		instance.first->setPath(vpath);
	}
	editor->change();
}

void PropertyBox::changeRadius(double d)
{
	beginChanging();
	indicateSimilarities(radiusL);
	auto instances = editor->getSelection()->getInstances();
	for (auto &instance : instances) {
		VolumetricPath vpath = instance.first->getPath();
		vpath.setMaxRadius(d);
		instance.first->setPath(vpath);
	}
	editor->change();
}

void PropertyBox::changeMinRadius(double d)
{
	beginChanging();
	indicateSimilarities(minRadiusL);
	auto instances = editor->getSelection()->getInstances();
	for (auto &instance : instances) {
		VolumetricPath vpath = instance.first->getPath();
		vpath.setMinRadius(d);
		instance.first->setPath(vpath);
	}
	editor->change();
}

void PropertyBox::changeRadiusCurve(pg::Spline &spline)
{
	beginChanging();
	auto instances = editor->getSelection()->getInstances();
	for (auto &instance : instances) {
		VolumetricPath vp = instance.first->getPath();
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
	auto instances = editor->getSelection()->getInstances();
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
	auto instances = editor->getSelection()->getInstances();
	for (auto &instance : instances)
		instance.first->setMaterial(pg::Stem::Inner, id);
	editor->change();
	finishChanging();
}

void PropertyBox::beginChanging()
{
	if (!changing) {
		StemSelection *selection = editor->getSelection();
		saveStem = SaveStem(selection);
		saveStem.execute();
	}
	changing = true;
}

void PropertyBox::finishChanging()
{
	if (changing && !saveStem.isSameAsCurrent()) {
		saveStem.setNewSelection();
		editor->add(saveStem);
	}
	changing = false;
}

void PropertyBox::enable(bool enable)
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

void PropertyBox::indicateDifferences(QWidget *widget)
{
	widget->setStyleSheet("font-weight:bold;");
}

void PropertyBox::indicateSimilarities(QWidget *widget)
{
	widget->setStyleSheet("");
}

void PropertyBox::bind(Editor *editor, CurveEditor *curveEditor)
{
	this->editor = editor;
	this->curveEditor = curveEditor;
	connect(editor, SIGNAL(selectionChanged()), this,
		SLOT(fill()));
	connect(curveEditor, SIGNAL(curveChanged(pg::Spline, QString)), this,
		SLOT(setCurve(pg::Spline, QString)));
	connect(curveEditor, SIGNAL(editingFinished()), this,
		SLOT(finishChanging()));
}

QSize PropertyBox::sizeHint() const
{
	return QSize(400, 200);
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
	auto instances = editor->getSelection()->getInstances();
	pg::Stem *stem = nullptr;
	if (!instances.empty()) {
		stem = instances.rbegin()->first;
		QString name = button->getName();
		curveEditor->setCurve(stem->getPath().getRadius(), name);
	}
}

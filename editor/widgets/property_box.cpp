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
#include "../history/stem_selection_state.h"
#include <iterator>

using pg::VolumetricPath;
using pg::Spline;
using std::next;
using std::prev;

PropertyBox::PropertyBox(SharedResources *shared, QWidget *parent) :
	QWidget(parent), memorize(nullptr)
{
	this->shared = shared;
	curveEditor = nullptr;
	selectedCurve = nullptr;

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setSizeConstraint(QLayout::SetMinimumSize);
	layout->setSpacing(0);
	layout->setMargin(0);
	createStemBox(layout);
	layout->addStretch(1);
}

/**
 * Create a container that stores properties that only affect the current stem.
 */
void PropertyBox::createStemBox(QVBoxLayout *layout)
{
	stemG = new QGroupBox(tr("Stem"));
	QVBoxLayout *groupLayout = new QVBoxLayout(stemG);

	stemT = new QTableWidget(this);
	stemT->setRowCount(4);
	stemT->setColumnCount(3);

	radiusL = new QLabel(tr("Radius"));
	radiusV = new QDoubleSpinBox;
	radiusB = new CurveButton("Radius", shared, this);
	radiusV->setSingleStep(0.001);
	radiusV->setDecimals(3);

	divisionL = new QLabel(tr("Divisions"));
	divisionV = new QSpinBox;
	divisionV->setMinimum(1);

	resolutionL = new QLabel(tr("Resolution"));
	resolutionV = new QSpinBox;
	resolutionV->setMinimum(5);

	degreeL = new QLabel(tr("Degree"));
	degreeV = new QComboBox;
	degreeV->addItem(QString("Linear"));
	degreeV->addItem(QString("Cubic"));

	{
		int row = 0;
		stemT->setCellWidget(row, 0, radiusL);
		stemT->setCellWidget(row, 1, radiusV);
		stemT->setCellWidget(row, 2, createCenteredWidget(radiusB));
		stemT->setCellWidget(++row, 0, resolutionL);
		stemT->setCellWidget(row, 1, resolutionV);
		stemT->setCellWidget(++row, 0, divisionL);
		stemT->setCellWidget(row, 1, divisionV);
		stemT->setCellWidget(++row, 0, degreeL);
		stemT->setCellWidget(row, 1, degreeV);
	}

	configureTable(stemT);
	groupLayout->addWidget(stemT);
	/* Show and hide methods are neeeded for hidden opengl widgets to
	 * initialize properly. */
	stemG->show();
	stemG->hide();
	stemG->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	layout->addWidget(stemG);
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
		stemG->hide();
		curveEditor->setEnabled(false);
	} else {
		pg::Stem *lastStem = instances.rbegin()->first;
		auto nextIt = next(instances.begin());
		
		/* TODO make curve bold if radius curves are different. */
		radiusB->blockSignals(true);
		radiusB->setCurve(lastStem->getPath().getRadius());
		radiusB->blockSignals(false);

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
		resolutionV->setValue(lastStem->getResolution());
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
		divisionV->setValue(lastStem->getPath().getResolution());
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
		radiusV->setValue(lastStem->getPath().getMaxRadius());
		radiusV->blockSignals(false);

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
		switch(lastStem->getPath().getSpline().getDegree()) {
		case 1:
			degreeV->setCurrentIndex(0);
			break;
		case 3:
			degreeV->setCurrentIndex(1);
			break;
		}
		degreeV->blockSignals(false);

		if (selectedCurve)
			selectedCurve->select();

		curveEditor->setEnabled(true);
		stemG->show();
		stemG->blockSignals(false);
	}
}

void PropertyBox::changePathDegree(int i)
{
	beginChanging(degreeV);
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
	beginChanging(resolutionV);
	indicateSimilarities(resolutionL);
	auto instances = editor->getSelection()->getInstances();
	for (auto &instance : instances)
		instance.first->setResolution(i);
	editor->change();
}

void PropertyBox::changeDivisions(int i)
{
	beginChanging(divisionV);
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
	beginChanging(radiusV);
	indicateSimilarities(radiusL);
	auto instances = editor->getSelection()->getInstances();
	for (auto &instance : instances) {
		VolumetricPath vpath = instance.first->getPath();
		vpath.setMaxRadius(d);
		instance.first->setPath(vpath);
	}
	editor->change();
}

void PropertyBox::changeRadiusCurve(pg::Spline &spline)
{
	beginChanging(curveEditor);
	auto instances = editor->getSelection()->getInstances();
	for (auto &instance : instances) {
		VolumetricPath vp = instance.first->getPath();
		vp.setRadius(spline);
		instance.first->setPath(vp);
	}
	editor->change();
}

void PropertyBox::beginChanging(QWidget *widget)
{
	if (widget->hasFocus()) {
		if (!changing) {
			StemSelection *selection = editor->getSelection();
			memorize = MemorizeStem(selection);
			memorize.execute();
		}
		changing = true;
	}
}

void PropertyBox::finishChanging()
{
	if (changing && !memorize.isSameAsCurrent()) {
		History *history = editor->getHistory();
		StemSelection *selection = editor->getSelection();
		history->add(memorize, StemSelectionState(selection));
	}
	changing = false;
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
	this->curveEditor = curveEditor;
	this->editor = editor;

	connect(editor, SIGNAL(selectionChanged()), this,
		SLOT(fill()));

	connect(curveEditor, SIGNAL(curveChanged(pg::Spline, QString)), this,
		SLOT(setCurve(pg::Spline, QString)));
	connect(radiusB, SIGNAL(selected(CurveButton *)), this,
		SLOT(toggleCurve(CurveButton *)));

	connect(resolutionV, SIGNAL(editingFinished()), this,
		SLOT(finishChanging()));
	connect(divisionV, SIGNAL(editingFinished()), this,
		SLOT(finishChanging()));
	connect(radiusV, SIGNAL(editingFinished()), this,
		SLOT(finishChanging()));
	connect(curveEditor, SIGNAL(editingFinished()), this,
		SLOT(finishChanging()));

	connect(degreeV, SIGNAL(currentIndexChanged(int)), this,
		SLOT(changePathDegree(int)));
	connect(resolutionV, SIGNAL(valueChanged(int)), this,
		SLOT(changeResolution(int)));
	connect(divisionV, SIGNAL(valueChanged(int)), this,
		SLOT(changeDivisions(int)));
	connect(radiusV, SIGNAL(valueChanged(double)), this,
		SLOT(changeRadius(double)));
}

/**
 * Determine the width of each column and what the height of the table is.
 * Since the table height is fixed, hiding rows requires setting a new
 * fixed height.
 */
void PropertyBox::configureTable(QTableWidget *table)
{
	table->horizontalHeader()->hide();
	table->verticalHeader()->hide();
	table->setShowGrid(false);
	table->setFocusPolicy(Qt::NoFocus);
	table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	table->setSelectionMode(QAbstractItemView::NoSelection);

	for (int row = 0; row < table->rowCount(); row++) {
		auto rightCell = table->cellWidget(row, 1);
		auto leftCell = table->cellWidget(row, 0);
		const char *name = rightCell->metaObject()->className();

		bool spinBox = strcmp(name, "QSpinBox") == 0;
		bool doubleSpinBox = strcmp(name, "QDoubleSpinBox") == 0;
		if (spinBox || doubleSpinBox)
			table->setRowHeight(row, 24);
		else
			table->resizeRowToContents(row);

		if (leftCell != nullptr)
			leftCell->setFixedHeight(24);
	}

	{
		QHeaderView *header = table->horizontalHeader();
		if (table->columnCount() == 3)
			header->setSectionResizeMode(2, QHeaderView::Fixed);
		header->setSectionResizeMode(1, QHeaderView::Stretch);
		header->setSectionResizeMode(0, QHeaderView::Fixed);
		header->resizeSection(2, 30);
	}

	{
		int height = 0;
		for (int i = 0; i < table->rowCount(); i++)
			height += table->rowHeight(i);
		table->setFixedHeight(height);
		table->resizeColumnToContents(0);
	}
}

QWidget *PropertyBox::createCenteredWidget(QWidget *widget)
{
	QWidget *w = new QWidget(this);
	QHBoxLayout *b = new QHBoxLayout(w);
	b->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
	b->setMargin(0);
	b->addWidget(widget);
	return w;
}

QSize PropertyBox::sizeHint() const
{
	return QSize(300, 0);
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
	if (!instances.empty())
		stem = instances.rbegin()->first;
	curveEditor->setCurve(stem->getPath().getRadius(), button->getName());
}

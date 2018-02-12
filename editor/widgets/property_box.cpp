/* Plant Genererator
 * Copyright (C) 2016-2017  Floris Creyf
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

PropertyBox::PropertyBox(SharedResources *shared, QWidget *parent) :
	QWidget(parent)
{
	this->shared = shared;
	curveEditor = nullptr;
	activeCurve = nullptr;

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setSizeConstraint(QLayout::SetMinimumSize);
	layout->setSpacing(0);
	layout->setMargin(0);
	createLocalBox(layout);
	layout->addStretch(1);
}

void PropertyBox::createLocalBox(QVBoxLayout *layout)
{
	localGroup = new QGroupBox(tr("Stem"));
	QVBoxLayout *groupLayout = new QVBoxLayout(localGroup);
	local = new QTableWidget(this);

	radius = new QDoubleSpinBox;
	radiusCB = new CurveButton("Radius", shared, this);
	resolution = new QSpinBox;
	divisions = new QSpinBox;
	degree = new QComboBox;

	local->setRowCount(4);
	local->setColumnCount(3);
	radius->setSingleStep(0.01);
	divisions->setMinimum(1);
	resolution->setMinimum(5);
	degree->addItem(QString("Linear"));
	degree->addItem(QString("Cubic"));

	{
		int i = 0;
		local->setCellWidget(i, 0, new QLabel(tr("Radius")));
		local->setCellWidget(i, 1, radius);
		local->setCellWidget(i, 2, createCenteredWidget(radiusCB));
		local->setCellWidget(++i, 0, new QLabel(tr("Resolution")));
		local->setCellWidget(i, 1, resolution);
		local->setCellWidget(++i, 0, new QLabel(tr("Divisions")));
		local->setCellWidget(i, 1, divisions);
		local->setCellWidget(++i, 0, new QLabel(tr("Degree")));
		local->setCellWidget(i, 1, degree);
	}

	configureTable(local);
	groupLayout->addWidget(local);
	localGroup->hide();
	localGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	layout->addWidget(localGroup);
}

void PropertyBox::configureTable(QTableWidget *table)
{
	table->horizontalHeader()->hide();
	table->verticalHeader()->hide();
	table->setShowGrid(false);
	table->setFocusPolicy(Qt::NoFocus);
	table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	table->setSelectionMode(QAbstractItemView::NoSelection);

	for (int i = 0; i < table->rowCount(); i++) {
		auto r = table->cellWidget(i, 1);
		auto l = table->cellWidget(i, 0);
		const char *name = r->metaObject()->className();

		bool spinBox = strcmp(name, "QSpinBox") == 0;
		bool doubleSpinBox = strcmp(name, "QDoubleSpinBox") == 0;
		if (spinBox || doubleSpinBox)
			table->setRowHeight(i, 24);
		else
			table->resizeRowToContents(i);

		if (l != nullptr)
			l->setFixedHeight(24);
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
		int h = 0;
		for (int i = 0; i < table->rowCount(); i++)
			h += table->rowHeight(i);
		table->setFixedHeight(h);
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
		radiusCB->setCurve(spline);
		changeRadiusCurve(spline);
	}
}

void PropertyBox::toggleCurve(CurveButton *button)
{
	activeCurve = button;
	pg::Stem *stem = editor->getSelectedStem();
	curveEditor->setCurve(stem->getPath().getRadius(), button->getName());
}

void PropertyBox::fill()
{
	pg::Stem *stem = editor->getSelectedStem();

	if (!stem) {
		localGroup->hide();
		curveEditor->setEnabled(false);
	} else {
		pg::VolumetricPath vpath = stem->getPath();

		radiusCB->blockSignals(true);
		radiusCB->setCurve(vpath.getRadius());
		radiusCB->blockSignals(false);

		resolution->blockSignals(true);
		resolution->setValue(stem->getResolution());
		resolution->blockSignals(false);

		divisions->blockSignals(true);
		divisions->setValue(vpath.getResolution());
		divisions->blockSignals(false);

		radius->blockSignals(true);
		radius->setValue(vpath.getMaxRadius());
		radius->blockSignals(false);

		degree->blockSignals(true);
		switch(vpath.getSpline().getDegree()) {
		case 1:
			degree->setCurrentIndex(0);
			break;
		case 3:
			degree->setCurrentIndex(1);
			break;
		}
		degree->blockSignals(false);

		if (activeCurve)
			activeCurve->select();

		curveEditor->setEnabled(true);
		divisions->setEnabled(true);
		localGroup->show();
		localGroup->blockSignals(false);
	}
}

void PropertyBox::changePathDegree(int i)
{
	beginChanging(degree);
	int degree = i == 1 ? 3 : 1;
	pg::Stem *stem = editor->getSelectedStem();
	pg::VolumetricPath path = stem->getPath();
	pg::Spline spline = path.getSpline();
	if (spline.getDegree() != degree) {
		int selectedPoint = editor->getSelectedPoint();
		selectedPoint = spline.adjust(degree, selectedPoint);
		editor->setSelectedPoint(selectedPoint);
		pg::Vec3 p = spline.getControls()[selectedPoint];
		p += stem->getLocation();
	}
	path.setSpline(spline);
	stem->setPath(path);
	editor->change();
	finishChanging();
}

void PropertyBox::changeResolution(int i)
{
	beginChanging(resolution);
	pg::Stem *stem = editor->getSelectedStem();
	stem->setResolution(i);
	editor->change();
}

void PropertyBox::changeDivisions(int i)
{
	beginChanging(divisions);
	pg::Stem *stem = editor->getSelectedStem();
	pg::VolumetricPath vpath = stem->getPath();
	vpath.setResolution(i);
	stem->setPath(vpath);
	editor->change();
}

void PropertyBox::changeRadius(double d)
{
	beginChanging(radius);
	pg::Stem *stem = editor->getSelectedStem();
	pg::VolumetricPath vpath = stem->getPath();
	vpath.setMaxRadius(d);
	stem->setPath(vpath);
	editor->change();
}

void PropertyBox::changeRadiusCurve(pg::Spline &spline)
{
	beginChanging(curveEditor);
	pg::Stem *stem = editor->getSelectedStem();
	pg::VolumetricPath vp = stem->getPath();
	vp.setRadius(spline);
	stem->setPath(vp);
	editor->change();
}

void PropertyBox::beginChanging(QWidget *widget)
{
	if (widget->hasFocus()) {
		if (!changing) {
			History *history = editor->getHistory();
			pg::Stem *stem = editor->getSelectedStem();
			int point = editor->getSelectedPoint();
			history->add(stem, point);
		}
		changing = true;
	}
}

void PropertyBox::finishChanging()
{
	changing = false;
}

void PropertyBox::bind(Editor *editor, CurveEditor *curveEditor)
{
	this->curveEditor = curveEditor;
	this->editor = editor;

	connect(editor, SIGNAL(selectionChanged()), this, SLOT(fill()));

	connect(curveEditor, SIGNAL(curveChanged(pg::Spline, QString)), this,
		SLOT(setCurve(pg::Spline, QString)));
	connect(radiusCB, SIGNAL(selected(CurveButton *)), this,
		SLOT(toggleCurve(CurveButton *)));

	connect(resolution, SIGNAL(editingFinished()), this,
		SLOT(finishChanging()));
	connect(divisions, SIGNAL(editingFinished()), this,
		SLOT(finishChanging()));
	connect(radius, SIGNAL(editingFinished()), this,
		SLOT(finishChanging()));
	connect(curveEditor, SIGNAL(editingFinished()), this,
		SLOT(finishChanging()));

	connect(degree, SIGNAL(currentIndexChanged(int)), this,
		SLOT(changePathDegree(int)));
	connect(resolution, SIGNAL(valueChanged(int)), this,
		SLOT(changeResolution(int)));
	connect(divisions, SIGNAL(valueChanged(int)), this,
		SLOT(changeDivisions(int)));
	connect(radius, SIGNAL(valueChanged(double)), this,
		SLOT(changeRadius(double)));
}

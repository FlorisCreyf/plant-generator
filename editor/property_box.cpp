/* TreeMaker: 3D tree model editor
 * Copyright (C) 2016-2017  Floris Creyf
 *
 * TreeMaker is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TreeMaker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "property_box.h"
#include <stdio.h>

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
	createGlobalBox(layout);
	createLocalBox(layout);
	layout->addStretch(1);
}

void PropertyBox::createGlobalBox(QVBoxLayout *layout)
{
	QGroupBox *globalGroup = new QGroupBox(tr("Tree"));
	QVBoxLayout *groupLayout = new QVBoxLayout(globalGroup);
	global = new QTableWidget(this);
	crownBaseHeight = new QDoubleSpinBox;
	apicalDominance = new QDoubleSpinBox;
	global->setRowCount(2);
	global->setColumnCount(2);
	apicalDominance->setEnabled(false);
	crownBaseHeight->setEnabled(false);
	global->setCellWidget(0, 0, new QLabel(tr("Crown Base Height")));
	global->setCellWidget(0, 1, crownBaseHeight);
	global->setCellWidget(1, 0, new QLabel(tr("Apical Dominance")));
	global->setCellWidget(1, 1, apicalDominance);
	configureTable(global);
	groupLayout->addStretch(1);
	groupLayout->addWidget(global);
	globalGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	layout->addWidget(globalGroup);
}

void PropertyBox::createLocalBox(QVBoxLayout *layout)
{
	localGroup = new QGroupBox(tr("Branch"));
	QVBoxLayout *groupLayout = new QVBoxLayout(localGroup);
	local = new QTableWidget(this);
	resolution = new QSpinBox;
	radius = new QDoubleSpinBox;
	radiusCB = new CurveButton("Radius", shared, this);
	sections = new QSpinBox;
	branches = new QDoubleSpinBox;
	local->setRowCount(4);
	local->setColumnCount(3);
	radius->setSingleStep(0.01);
	sections->setMinimum(2);
	resolution->setMinimum(5);
	local->setCellWidget(0, 0, new QLabel(tr("Radius")));
	local->setCellWidget(0, 1, radius);
	local->setCellWidget(0, 2, createCenteredWidget(radiusCB));
	local->setCellWidget(1, 0, new QLabel(tr("Resolution")));
	local->setCellWidget(1, 1, resolution);
	local->setCellWidget(2, 0, new QLabel(tr("Cross Sections")));
	local->setCellWidget(2, 1, sections);
	local->setCellWidget(3, 0, new QLabel(tr("Branches / Unit")));
	local->setCellWidget(3, 1, branches);
	configureTable(local);
	groupLayout->addStretch(1);
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
	table->setMaximumHeight(24*table->rowCount());
	for (int i = 0; i < table->rowCount(); i++)
		table->setRowHeight(i, 24);

	{
		QHeaderView *header = table->horizontalHeader();
		if (table->columnCount() == 3)
			header->setSectionResizeMode(2, QHeaderView::Fixed);
		header->setSectionResizeMode(1, QHeaderView::Stretch);
		header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
		header->resizeSection(2, 30);
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

void PropertyBox::setCurve(vector<TMvec3> controls, QString name)
{
	if (name == "Radius") {
		radiusCB->setControls(controls);
		editor->changeRadiusCurve(controls);
	}
}

void PropertyBox::toggleCurve(CurveButton *w)
{
	activeCurve = w;
	curveEditor->setCurve(w->getControls(), w->getName());
}

void PropertyBox::fill(TMtree tree, int branch)
{
	crownBaseHeight->setValue(tmGetCrownBaseHeight(tree));

	if (branch < 0) {
		localGroup->hide();
		curveEditor->setEnabled(false);
		return;
	} else {
		localGroup->show();
		curveEditor->setEnabled(true);
	}
	
	resolution->setValue(tmGetResolution(tree, branch));
	sections->setValue(tmGetCrossSections(tree, branch));
	radius->setValue(tmGetRadius(tree, branch));
	branches->setValue(tmGetBranchDensity(tree, branch));
	fillCurveButtons(tree, branch);

	if (activeCurve)
		activeCurve->select();

	if (tmIsTerminalBranch(tree, branch))
		resolution->setEnabled(false);
	else
		resolution->setEnabled(true);
}

void PropertyBox::fillCurveButtons(TMtree tree, int branch)
{
	int size;
	TMvec3 *curve;
	tmGetRadiusCurve(tree, branch, &curve, &size);
	radiusCB->setControls(curve, size);
}

void PropertyBox::bind(Editor *editor, CurveEditor *curveEditor)
{
	this->curveEditor = curveEditor;
	this->editor = editor;

	connect(editor, SIGNAL(selectionChanged(TMtree, int)), this,
			SLOT(fill(TMtree, int)));
	connect(resolution, SIGNAL(valueChanged(int)), editor,
			SLOT(changeResolution(int)));
	connect(sections, SIGNAL(valueChanged(int)), editor,
			SLOT(changeSections(int)));
	connect(radius, SIGNAL(valueChanged(double)), editor,
			SLOT(changeRadius(double)));
	connect(branches, SIGNAL(valueChanged(double)), editor,
			SLOT(changeBranchDensity(double)));

	bindCurveEditor();
}

void PropertyBox::bindCurveEditor()
{
	connect(curveEditor, SIGNAL(curveChanged(vector<TMvec3>, QString)),
			this, SLOT(setCurve(vector<TMvec3>, QString)));
	connect(radiusCB, SIGNAL(selected(CurveButton *)), this,
			SLOT(toggleCurve(CurveButton *)));
}

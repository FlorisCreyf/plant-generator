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
	global->setRowCount(0);
	global->setColumnCount(2);
	configureTable(global);
	groupLayout->addStretch(1);
	groupLayout->addWidget(global);
	globalGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	layout->addWidget(globalGroup);
}

void PropertyBox::createLocalBox(QVBoxLayout *layout)
{
	localGroup = new QGroupBox(tr("Stem"));
	QVBoxLayout *groupLayout = new QVBoxLayout(localGroup);
	local = new QTableWidget(this);
	resolution = new QSpinBox;
	radius = new QDoubleSpinBox;
	radiusCB = new CurveButton("Radius", shared, this);
	sections = new QSpinBox;
	stems = new QDoubleSpinBox;
	base = new QDoubleSpinBox;
	local->setRowCount(5);
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
	local->setCellWidget(3, 0, new QLabel(tr("Stem Density")));
	local->setCellWidget(3, 1, stems);
	local->setCellWidget(4, 0, new QLabel(tr("Base Length")));
	local->setCellWidget(4, 1, base);
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

void PropertyBox::setCurve(std::vector<treemaker::Vec3> controls, QString name)
{
	if (name == "Radius") {
		radiusCB->setControls(controls);
		editor->changeRadiusCurve(controls);
	}
}

void PropertyBox::toggleCurve(CurveButton *button)
{
	activeCurve = button;
	curveEditor->setCurve(button->getControls(), button->getName());
}

void PropertyBox::fill(treemaker::Tree &tree, int stem)
{
	if (stem < 0) {
		localGroup->hide();
		curveEditor->setEnabled(false);
		return;
	} else {
		localGroup->show();
		curveEditor->setEnabled(true);
	}

	resolution->setValue(tree.getResolution(stem));
	sections->setValue(tree.getGeneratedPathSize(stem));
	radius->setValue(tree.getRadius(stem));
	stems->setValue(tree.getStemDensity(stem));
	base->setValue(tree.getBaseLength(stem));
	fillCurveButtons(tree, stem);

	if (activeCurve)
		activeCurve->select();
}

void PropertyBox::fillCurveButtons(treemaker::Tree &tree, int stem)
{
	std::vector<treemaker::Vec3> curve(tree.getRadiusCurveSize(stem));
	tree.getRadiusCurve(stem, &curve[0]);
	radiusCB->setControls(curve);
}

void PropertyBox::bind(Editor *editor, CurveEditor *curveEditor)
{
	this->curveEditor = curveEditor;
	this->editor = editor;

	connect(editor, SIGNAL(selectionChanged(treemaker::Tree &, int)), this,
			SLOT(fill(treemaker::Tree &, int)));
	connect(resolution, SIGNAL(valueChanged(int)), editor,
			SLOT(changeResolution(int)));
	connect(sections, SIGNAL(valueChanged(int)), editor,
			SLOT(changeSections(int)));
	connect(radius, SIGNAL(valueChanged(double)), editor,
			SLOT(changeRadius(double)));
	connect(stems, SIGNAL(valueChanged(double)), editor,
			SLOT(changeStemDensity(double)));
	connect(base, SIGNAL(valueChanged(double)), editor,
			SLOT(changeBaseLength(double)));

	bindCurveEditor();
}

void PropertyBox::bindCurveEditor()
{
	connect(curveEditor, SIGNAL(
			curveChanged(std::vector<treemaker::Vec3>, QString)),
			this,
			SLOT(setCurve(std::vector<treemaker::Vec3>, QString)));
	
	connect(radiusCB, SIGNAL(selected(CurveButton *)), this, 
			SLOT(toggleCurve(CurveButton *)));
}

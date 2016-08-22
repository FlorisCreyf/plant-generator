/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "property_box.h"
#include <stdio.h>

PropertyBox::PropertyBox(QWidget *parent) : QWidget(parent)
{
	QHeaderView *header;
	QVBoxLayout *layout = new QVBoxLayout(this);

	resolution = new QSpinBox;
	radius = new QDoubleSpinBox;
	radiusCB = new CurveButtonWidget("Radius", this);
	sections = new QSpinBox;
	branches = new QSpinBox;
	branchesCB = new CurveButtonWidget("Distribution", this);

	radius->setSingleStep(0.01);
	sections->setMinimum(2);
	resolution->setMinimum(5);

	table = new QTableWidget(this);
	table->setRowCount(5);
	table->setColumnCount(3);
	table->setCellWidget(0, 0, new QLabel(tr("Radius")));
	table->setCellWidget(0, 1, radius);
	table->setCellWidget(0, 2, createCenteredWidget(radiusCB));
	table->setCellWidget(1, 0, new QLabel(tr("Resolution")));
	table->setCellWidget(1, 1, resolution);
	table->setCellWidget(2, 0, new QLabel(tr("Cross Sections")));
	table->setCellWidget(2, 1, sections);
	table->setCellWidget(3, 0, new QLabel(tr("Branches")));
	table->setCellWidget(3, 1, branches);
	table->setCellWidget(3, 2, createCenteredWidget(branchesCB));
	table->horizontalHeader()->hide();
	table->verticalHeader()->hide();
	table->setShowGrid(false);
	table->setFocusPolicy(Qt::NoFocus);
	table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	table->setSelectionMode(QAbstractItemView::NoSelection);
	for (int i = 0; i < 5; i++)
		table->setRowHeight(i, 26);

	header = table->horizontalHeader();
	header->setSectionResizeMode(2, QHeaderView::Fixed);
	header->setSectionResizeMode(1, QHeaderView::Stretch);
	header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	header->resizeSection(2, 30);

	layout->addWidget(table);
	table->hide();
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

void PropertyBox::setCurve(vector<tm_vec3> controls, QString name)
{
	if (name == "Radius") {
		radiusCB->getCurveButton()->setControls(controls);
		sceneEditor->changeRadiusCurve(controls);
	}
}

void PropertyBox::toggleCurve(CurveButtonWidget *w)
{
	activeCurve = w;
	curveEditor->setCurve(w->getCurveButton()->getControls(), w->getName());
}

void PropertyBox::fill(tm_tree tree, int branch)
{
	if (branch < 0) {
		table->hide();
		curveEditor->setEnabled(false);
		return;
	}

	resolution->setValue(tm_get_resolution(tree, branch));
	sections->setValue(tm_get_cross_sections(tree, branch));
	radius->setValue(tm_get_radius(tree, branch));
	fillCurveButtons(tree, branch);

	table->show();
	curveEditor->setEnabled(true);

	if (activeCurve)
		activeCurve->select();

	if (tm_is_terminal_branch(tree, branch))
		resolution->setEnabled(false);
	else
		resolution->setEnabled(true);
}

void PropertyBox::fillCurveButtons(tm_tree tree, int branch)
{
	int size;
	tm_vec3 *l;
	tm_get_radius_curve(tree, branch, &l, &size);
	radiusCB->getCurveButton()->setControls(l, size);
}

void PropertyBox::bind(SceneEditor *sceneEditor, CurveEditor *curveEditor)
{
	this->curveEditor = curveEditor;
	this->sceneEditor = sceneEditor;

	connect(sceneEditor, SIGNAL(selectionChanged(tm_tree, int)), this,
			SLOT(fill(tm_tree, int)));
	connect(resolution, SIGNAL(valueChanged(int)), sceneEditor,
			SLOT(changeResolution(int)));
	connect(sections, SIGNAL(valueChanged(int)), sceneEditor,
			SLOT(changeSections(int)));
	connect(radius, SIGNAL(valueChanged(double)), sceneEditor,
			SLOT(changeRadius(double)));

	bindCurveEditor();
}

void PropertyBox::bindCurveEditor()
{
	connect(curveEditor, SIGNAL(curveChanged(vector<tm_vec3>, QString)),
			this, SLOT(setCurve(vector<tm_vec3>, QString)));

	connect(radiusCB, SIGNAL(selected(CurveButtonWidget *)), this,
			SLOT(toggleCurve(CurveButtonWidget *)));
}

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
	QVBoxLayout *l = new QVBoxLayout(this);

	resolution = new QSpinBox;
	radius = new QDoubleSpinBox;
	radiusCB = new CurveButtonWidget("Radius");
	sections = new QSpinBox;
	branches = new QSpinBox;
	branchesCB = new CurveButtonWidget("Distribution");

	radius->setSingleStep(0.01);
	sections->setMinimum(2);
	resolution->setMinimum(5);

	t = new QTableWidget(this);
	t->setRowCount(5);
	t->setColumnCount(3);
	t->setCellWidget(0, 0, new QLabel(tr("Radius")));
	t->setCellWidget(0, 1, radius);
	t->setCellWidget(0, 2, createCenteredWidget(radiusCB));
	t->setCellWidget(1, 0, new QLabel(tr("Resolution")));
	t->setCellWidget(1, 1, resolution);
	t->setCellWidget(2, 0, new QLabel(tr("Cross Sections")));
	t->setCellWidget(2, 1, sections);
	t->setCellWidget(3, 0, new QLabel(tr("Branches")));
	t->setCellWidget(3, 1, branches);
	t->setCellWidget(3, 2, createCenteredWidget(branchesCB));
	t->horizontalHeader()->hide();
	t->verticalHeader()->hide();
	t->setShowGrid(false);
	t->setFocusPolicy(Qt::NoFocus);
	t->setEditTriggers(QAbstractItemView::NoEditTriggers);
	t->setSelectionMode(QAbstractItemView::NoSelection);
	for (int i = 0; i < 5; i++)
		t->setRowHeight(i, 26);

	header = t->horizontalHeader();
	header->setSectionResizeMode(2, QHeaderView::Fixed);
	header->setSectionResizeMode(1, QHeaderView::Stretch);
	header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	header->resizeSection(2, 30);

	l->addWidget(t);
	t->hide();
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

void PropertyBox::setCurve(std::vector<tm_vec3> controls, QString name)
{
	if (name == "Radius") {
		radiusCB->setControls(controls);
		emit radiusCurveChanged(controls);
	}
}

void PropertyBox::fill(tm_tree tree, int branch)
{
	if (branch < 0) {
		t->hide();
		emit isEnabled(false);
		return;
	}

	resolution->setValue(tm_get_resolution(tree, branch));
	sections->setValue(tm_get_cross_sections(tree, branch));
	radius->setValue(tm_get_radius(tree, branch));
	fillCurveButtons(tree, branch);

	t->show();
	emit isEnabled(true);

	if (tm_is_terminal_branch(tree, branch))
		resolution->setEnabled(false);
	else
		resolution->setEnabled(true);
}

void PropertyBox::fillCurveButtons(tm_tree tree, int branch)
{
	tm_vec3 *l;
	int size;
	std::vector<tm_vec3> controls;

	tm_get_radius_curve(tree, branch, &l, &size);
	controls.insert(controls.begin(), l, &l[size]);
	radiusCB->setControls(controls);
}

void PropertyBox::setSignals(SceneEditor *sceneEditor, CurveEditor *curveEditor)
{
	connect(sceneEditor, SIGNAL(selectionChanged(tm_tree, int)), this,
			SLOT(fill(tm_tree, int)));
	connect(resolution, SIGNAL(valueChanged(int)), sceneEditor,
			SLOT(changeResolution(int)));
	connect(sections, SIGNAL(valueChanged(int)), sceneEditor,
			SLOT(changeSections(int)));
	connect(radius, SIGNAL(valueChanged(double)), sceneEditor,
			SLOT(changeRadius(double)));

	connect(this, SIGNAL(isEnabled(bool)), curveEditor,
			SLOT(setEnabled(bool)));

	connect(radiusCB, SIGNAL(selected(std::vector<tm_vec3>, QString)),
			curveEditor, SLOT(setCurve(std::vector<tm_vec3>, QString)));
	connect(curveEditor, SIGNAL(curveChanged(std::vector<tm_vec3>, QString)),
			this, SLOT(setCurve(std::vector<tm_vec3>, QString)));
	connect(this, SIGNAL(radiusCurveChanged(std::vector<tm_vec3>)),
			sceneEditor, SLOT(changeRadiusCurve(std::vector<tm_vec3>)));
}

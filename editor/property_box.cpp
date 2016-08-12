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
	QVBoxLayout *l = new QVBoxLayout(this);

	QWidget *w = new QWidget(this);
	QHBoxLayout *b = new QHBoxLayout(w);
	radiusCurve = new CurveButtonWidget(w);
	b->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
	b->setMargin(0);
	b->addWidget(radiusCurve);

	resolution = new QSpinBox;
	radius = new QDoubleSpinBox;
	sections = new QSpinBox;

	radius->setSingleStep(0.01);
	sections->setMinimum(2);
	resolution->setMinimum(5);
	t = new QTableWidget(this);
	t->setRowCount(5);
	t->setColumnCount(3);
	t->setCellWidget(0, 0, new QLabel(tr("Radius")));
	t->setCellWidget(0, 1, radius);
	t->setCellWidget(0, 2, w);
	t->setCellWidget(1, 0, new QLabel(tr("Resolution")));
	t->setCellWidget(1, 1, resolution);
	t->setCellWidget(2, 0, new QLabel(tr("Cross Sections")));
	t->setCellWidget(2, 1, sections);
	t->horizontalHeader()->hide();
	t->verticalHeader()->hide();
	t->setShowGrid(false);
	t->setSelectionMode(QAbstractItemView::NoSelection);
	for (int i = 0; i < 5; i++)
		t->setRowHeight(i, 26);

	t->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
	t->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	t->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	t->horizontalHeader()->resizeSection(2, 30);

	l->addWidget(t);
	t->hide();
}

QSize PropertyBox::sizeHint() const
{
	return QSize(300, 0);
}

void PropertyBox::createSignals(ViewEditor *e)
{
	connect(e, SIGNAL(selectionChanged(bt_tree, int)), this,
			SLOT(fill(bt_tree, int)));
	connect(resolution, SIGNAL(valueChanged(int)), e,
			SLOT(changeResolution(int)));
	connect(sections, SIGNAL(valueChanged(int)), e,
			SLOT(changeSections(int)));
	connect(radius, SIGNAL(valueChanged(double)), e,
			SLOT(changeRadius(double)));
}

void PropertyBox::updateCurveButton(std::vector<bt_vec3> controls)
{
	radiusCurve->setControls(controls);
}

void PropertyBox::fill(bt_tree tree, int s)
{
	if (s < 0) {
		t->hide();
		emit isEnabled(false);
		return;
	}

	int terminal = bt_is_terminal_branch(tree, s);
	resolution->setValue(bt_get_resolution(tree, s));
	sections->setValue(bt_get_cross_sections(tree, s));
	radius->setValue(bt_get_radius(tree, s));

	t->show();
	emit isEnabled(true);
	if (terminal)
		resolution->setEnabled(false);
	else
		resolution->setEnabled(true);
}

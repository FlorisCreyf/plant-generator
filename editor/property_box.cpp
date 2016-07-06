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
	QVBoxLayout *l = new QVBoxLayout();
	QGroupBox *form = new QGroupBox(tr("Trunk"));
	QFormLayout *layout =  new QFormLayout;

	resolution = new QSpinBox;
	radius = new QDoubleSpinBox;
	sections = new QSpinBox;

	radius->setSingleStep(0.01);
	sections->setMinimum(2);
	resolution->setMinimum(5);

	layout->setVerticalSpacing(2);
	layout->addRow(new QLabel(tr("Radius")), radius);
	layout->addRow(new QLabel(tr("Resolution")), resolution);
	layout->addRow(new QLabel(tr("Cross sections")), sections);
	form->setLayout(layout);
	l->addWidget(form);
	setLayout(l);
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

void PropertyBox::fill(bt_tree tree, int s)
{
	resolution->setValue(bt_get_resolution(tree));
	sections->setValue(bt_get_cross_sections(tree));
	radius->setValue(bt_get_radius(tree));
}

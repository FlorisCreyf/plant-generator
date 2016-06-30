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
	layout->setVerticalSpacing(2);
	layout->addRow(new QLabel(tr("Radius")), new QDoubleSpinBox);
	layout->addRow(new QLabel(tr("Resolution")), new QSpinBox);
	layout->addRow(new QLabel(tr("Cross sections")), new QSpinBox);
	form->setLayout(layout);
	l->addWidget(form);
	setLayout(l);
}

QSize PropertyBox::sizeHint() const
{
	return QSize(300, 0);
}

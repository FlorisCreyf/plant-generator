/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef PROPERTY_BOX
#define PROPERTY_BOX

#include "view_editor.h"
#include <QtWidgets>

class ViewEditor;

class PropertyBox : public QWidget
{
	Q_OBJECT

public:
	PropertyBox(QWidget *parent = 0);
	QSize sizeHint() const;
	void createSignals(ViewEditor *e);

public slots:
	void fill(bt_tree tree, int s);

private:
	QDoubleSpinBox *radius;
	QSpinBox *resolution;
	QSpinBox *sections;
};

#endif /* PROPERTY_BOX */

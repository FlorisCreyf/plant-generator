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
#include "curve_button.h"
#include "curve_editor.h"
#include <QtWidgets>

class PropertyBox : public QWidget
{
	Q_OBJECT

public:
	PropertyBox(QWidget *parent = 0);
	QSize sizeHint() const;
	void setSignals(ViewEditor *sceneEditor, CurveEditor *curveEditor);

public slots:
	void fill(bt_tree tree, int branch);
	void setCurve(std::vector<bt_vec3> control, QString names);

signals:
	void isEnabled(bool enabled);
	void radiusCurveChanged(std::vector<bt_vec3>);

private:
	QTableWidget *t;
	QDoubleSpinBox *radius;
	CurveButtonWidget *radiusCB;
	QSpinBox *resolution;
	QSpinBox *sections;
	QSpinBox *branches;
	CurveButtonWidget *branchesCB;

	void fillCurveButtons(bt_tree tree, int branch);
	QWidget *createCenteredWidget(QWidget *);
};

#endif /* PROPERTY_BOX */

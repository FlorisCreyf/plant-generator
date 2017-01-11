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

#include "scene_editor.h"
#include "curve_button.h"
#include "curve_editor.h"
#include <QtWidgets>

class PropertyBox : public QWidget
{
	Q_OBJECT

public:
	PropertyBox(QWidget *parent = 0);
	QSize sizeHint() const;
	void bind(SceneEditor *sceneEditor, CurveEditor *curveEditor);

public slots:
	void fill(TMtree tree, int branch);
	void setCurve(vector<TMvec3> control, QString names);
	void toggleCurve(CurveButtonWidget *w);

signals:
	void isEnabled(bool enabled);
	void radiusCurveChanged(vector<TMvec3>);

private:
	CurveEditor *curveEditor;
	SceneEditor *sceneEditor;
	CurveButtonWidget *activeCurve;

	QTableWidget *global;
	QDoubleSpinBox *crownBaseHeight;
	QDoubleSpinBox *apicalDominance;

	QTableWidget *local;
	QGroupBox *localGroup;
	QDoubleSpinBox *radius;
	CurveButtonWidget *radiusCB;
	QSpinBox *resolution;
	QSpinBox *sections;
	QDoubleSpinBox *branches;

	void bindCurveEditor();
	void fillCurveButtons(TMtree tree, int branch);
	QWidget *createCenteredWidget(QWidget *);
	void configureTable(QTableWidget *);
};

#endif /* PROPERTY_BOX */

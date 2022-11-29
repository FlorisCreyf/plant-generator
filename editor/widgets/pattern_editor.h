/* Plant Generator
 * Copyright (C) 2020  Floris Creyf
 *
 * Plant Generator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Plant Generator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PATTERN_EDITOR_H
#define PATTERN_EDITOR_H

#include "editor.h"
#include "widgets.h"
#include "curve_editor.h"
#include "../commands/generate.h"
#include <string>

class PatternEditor : public QWidget {
	Q_OBJECT

	Editor *editor;
	KeyMap *keymap;
	SharedResources *shared;
	Generate *generate;

	ComboBox *nodeValue;
	QPushButton *childButton;
	QPushButton *siblingButton;
	QPushButton *removeButton;
	std::string name;

	enum {SMaxDepth, SSeed, ISSize};
	enum {SDensity, SDistance, SLength, SRadius, SAngleVariation, SNoise,
		SRadiusThreshold, SFork, SForkAngle, SForkScale,
		SInclineVariation, SPull, SRadiusVariation, SPointDensity,
		DSSize};
	SpinBox *isv[ISSize];
	DoubleSpinBox *dsv[DSSize];
	enum {LeavesPerNode, ILSize};
	enum {LDensity, LDistance, LRotation, LMinUp, LMaxUp, LLocalUp,
		LGlobalUp, LMinForward, LMaxForward, LPull, LScaleX, LScaleY,
		LScaleZ, DLSize};
	SpinBox *ilv[ILSize];
	DoubleSpinBox *dlv[DLSize];

	void createInterface();
	void createStemLeafFields();
	void createStemGroup(QBoxLayout *);
	void createLeafGroup(QBoxLayout *);
	void createNodeGroup(QBoxLayout *);
	void removeCurrent();
	void blockSignals(bool);
	void setEnabled(bool);
	void createCommand();
	void setFields(const pg::ParameterTree &, std::string);
	void setStemData(pg::StemData);
	void setLeafData(pg::LeafData);

	CurveEditor *curveEditor;
	ComboBox *curveDegree;
	ComboBox *curveType;
	ComboBox *curveNode;

	void createCurveGroup(QBoxLayout *);
	void setCurveFields();
	void selectCurve();
	void enableCurve(bool);
	void updateCurve(pg::Spline);
	void updateParameterTree(pg::Spline);

public:
	PatternEditor(SharedResources *shared, KeyMap *keymap, Editor *editor,
		QWidget *parent);
	QSize sizeHint() const;
	void change();
	void changeField(std::function<void(pg::StemData *)> function);
	void select();
	void addChildNode();
	void addSiblingNode();
	void removeNode();

public slots:
	void setFields();
};

#endif

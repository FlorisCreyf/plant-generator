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

#ifndef GENERATOR_EDITOR_H
#define GENERATOR_EDITOR_H

#include "editor.h"
#include "widgets.h"
#include "../commands/generate.h"
#include <string>

class GeneratorEditor : public QWidget {
	Q_OBJECT

	Editor *editor;
	Generate *generate;

	QGroupBox *nodeGroup;
	QGroupBox *rootGroup;
	QGroupBox *stemGroup;
	QGroupBox *leafGroup;
	ComboBox *nodeValue;
	QPushButton *childButton;
	QPushButton *siblingButton;
	QPushButton *removeButton;

	enum {RootLength, RootFork, RootForkAngle, RootThreshold, RootNoise,
		DRSize};
	enum {Seed, IRSize};
	SpinBox *irv[IRSize];
	DoubleSpinBox *drv[DRSize];
	enum {StemDensity, StemDistance, Length, Scale, AngleVariation, Noise,
		RadiusThreshold, Fork, ForkAngle, LeafDensity, LeafDistance,
		LeafRotation, MinUp, MaxUp, LocalUp, GlobalUp, MinForward,
		MaxForward, Pull, ScaleX, ScaleY, ScaleZ, DSize};
	enum {LeavesPerNode, ISize};
	DoubleSpinBox *dv[DSize];
	SpinBox *iv[ISize];

	void createInterface();
	void createNodeGroup(QBoxLayout *);
	void createRootGroup(QBoxLayout *);
	void removeCurrent();
	void blockSignals(bool);
	void setEnabled(bool);
	void beginChanging();
	void setFields(const pg::ParameterTree &, std::string);
	void setStemData(pg::StemData);
	void setLeafData(pg::LeafData);
	pg::StemData getRootData(pg::StemData);
	pg::StemData getStemData(pg::StemData);
	pg::LeafData getLeafData(pg::LeafData);

public:
	GeneratorEditor(Editor *editor, QWidget *parent);
	QSize sizeHint() const;

public slots:
	void change();
	void changeOnce();
	void setFields();
	void finishChanging();
	void select();
	void addChildNode();
	void addSiblingNode();
	void removeNode();

signals:
	void parameterTreeModified();
};

#endif

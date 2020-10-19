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

#include "form.h"
#include "editor.h"
#include "../commands/generate.h"
#include <string>

class GeneratorEditor : public Form {
	Q_OBJECT

	Editor *editor;
	Generate *generate;

	QGroupBox *group;
	QComboBox *nodeValue;
	QPushButton *childButton;
	QPushButton *siblingButton;
	QPushButton *removeButton;
	QLabel *seedLabel;
	QSpinBox *seedValue;
	QLabel *stemDensityLabel;
	QDoubleSpinBox *stemDensityValue;
	QLabel *stemStartLabel;
	QDoubleSpinBox *stemStartValue;
	QLabel *leafDensityLabel;
	QDoubleSpinBox *leafDensityValue;
	QLabel *leafStartLabel;
	QDoubleSpinBox *leafStartValue;
	QLabel *radiusThresholdLabel;
	QDoubleSpinBox *radiusThresholdValue;
	QLabel *leafScaleLabel[3];
	QDoubleSpinBox *leafScaleValue[3];
	QLabel *lengthFactorLabel;
	QDoubleSpinBox *lengthFactorValue;
	QLabel *arrangementLabel;
	QComboBox *arrangementValue;

	void createInterface();
	void removeCurrent();
	void blockSignals(bool);
	void enable(bool);
	void beginChanging();
	void setFields(const pg::DerivationTree &, std::string);

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
	void derivationModified();
};

#endif

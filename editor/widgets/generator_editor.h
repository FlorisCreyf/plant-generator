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

class GeneratorEditor : public Form {
	Q_OBJECT

	Editor *editor;

	QLabel *seedLabel;
	QSpinBox *seedValue;
	QLabel *depthLabel;
	QSpinBox *depthValue;
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
	QLabel *lengthFactorLabel;
	QDoubleSpinBox *lengthFactorValue;
	QLabel *arrangementLabel;
	QComboBox *arrangementValue;
	Generate *generate;

	void createInterface();
	void removeCurrent();
	void blockSignals(bool);
	void enable(bool);
	void beginChanging(QLabel *label);

public:
	GeneratorEditor(Editor *editor, QWidget *parent);
	QSize sizeHint() const;

public slots:
	void change();
	void setFields();
	void finishChanging();
	void changeSeed(int seed);
	void changeStemDensity(double density);
	void changeLeafDensity(double density);
	void changeStemStart(double start);
	void changeLeafStart(double start);
	void changeRadiusThreshold(double threshold);
	void changeLengthFactor(double factor);
	void changeDepth(int depth);
	void changeArrangement(int arrangement);
};

#endif

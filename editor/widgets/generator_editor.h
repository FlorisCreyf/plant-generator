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

	QGroupBox *nodeGroup;
	QGroupBox *stemGroup;
	QGroupBox *leafGroup;
	QComboBox *nodeValue;
	QSpinBox *seedValue;
	QPushButton *childButton;
	QPushButton *siblingButton;
	QPushButton *removeButton;

	enum {
		LeavesPerNode
	};
	enum {
		StemDensity,
		StemStart,
		LengthFactor,
		RadiusThreshold
	};
	enum {
		LeafDensity,
		LeafDistance,
		LeafScaleX,
		LeafScaleY,
		LeafScaleZ,
		LeafRotation,
		MinUp,
		MaxUp,
		MinDirection,
		MaxDirection
	};

	static const int dssize = 4;
	QDoubleSpinBox *dsv[dssize];
	QLabel *dsl[dssize];
	static const int dlsize = 10;
	QDoubleSpinBox *dlv[dlsize];
	QLabel *dll[dlsize];
	static const int ilsize = 1;
	QSpinBox *ilv[ilsize];
	QLabel *ill[ilsize];

	void createInterface();
	void createNodeGroup(QBoxLayout *);
	void createStemGroup(QBoxLayout *);
	void createLeafGroup(QBoxLayout *);
	QFormLayout *createForm(QGroupBox *);
	void removeCurrent();
	void blockSignals(bool);
	void enable(bool);
	void beginChanging();
	void setFields(const pg::ParameterTree &, std::string);
	void setStemData(pg::StemData);
	void setLeafData(pg::LeafData);
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

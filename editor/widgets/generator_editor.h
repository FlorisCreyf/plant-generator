/* Plant Generator
 * Copyright (C) 2021  Floris Creyf
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

class GeneratorWorkload : public QObject {
	Q_OBJECT

	pg::Scene *scene;

public:
	GeneratorWorkload(pg::Scene *scene);

public slots:
	void generate();

signals:
	void done();
};

class GeneratorEditor : public QWidget {
	Q_OBJECT

	QThread thread;
	Editor *editor;
	GeneratorWorkload *workload;

	enum {PrimaryRate, SecondaryRate, Suppression, SuppressionFalloff,
		SynthesisThreshold, SynthesisRate, Optimization, DSize};
	enum {Cycles, Nodes, Rays, Depth, Seed, ISize};

	QPushButton *startButton;
	QPushButton *toggleVolumeButton;
	SpinBox *iv[ISize];
	DoubleSpinBox *dv[DSize];

	void createInterface();
	void setValues();
	void change();

public:
	GeneratorEditor(Editor *editor, QWidget *parent);
	~GeneratorEditor();

public slots:
	void start();
	void end();

signals:
	void generate();
	void reset();
};

#endif

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

#include "generator_editor.h"
#include "form.h"

using pg::Generator;
using pg::Scene;

GeneratorEditor::GeneratorEditor(Editor *editor, QWidget *parent) :
	QWidget(parent), editor(editor), workload(nullptr)
{
	createInterface();
	this->workload = new GeneratorWorkload(editor->getScene());
	this->workload->moveToThread(&this->thread);
	connect(&this->thread, &QThread::finished,
		this->workload, &QObject::deleteLater);
	connect(this, &GeneratorEditor::generate,
		this->workload, &GeneratorWorkload::generate);
	connect(this->workload, &GeneratorWorkload::done,
		this, &GeneratorEditor::end);
	this->thread.start();
}

GeneratorEditor::~GeneratorEditor()
{
	thread.quit();
	thread.wait();
}

void GeneratorEditor::createInterface()
{
	for (int i = 0; i < DSize; i++) {
		this->dv[i] = new DoubleSpinBox(this);
		this->dv[i]->setSingleStep(0.01);
		this->dv[i]->setDecimals(3);
	}
	for (int i = 0; i < ISize; i++) {
		this->iv[i] = new SpinBox(this);
		this->iv[i]->setSingleStep(1);
		this->iv[i]->setRange(0, 1000);
	}

	Generator *g = &editor->getScene()->generator;
	QBoxLayout *layout = new QVBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(0);

	QGroupBox *group = createGroup("Properties");
	QFormLayout *form = createForm(group);
	this->dv[PrimaryRate]->setValue(g->primaryGrowthRate);
	form->addRow("Primary Growth", this->dv[PrimaryRate]);
	this->dv[SecondaryRate]->setSingleStep(0.001);
	this->dv[SecondaryRate]->setDecimals(4);
	this->dv[SecondaryRate]->setValue(g->secondaryGrowthRate);
	form->addRow("Secondary Growth", this->dv[SecondaryRate]);
	this->dv[Suppression]->setSingleStep(0.0001);
	this->dv[Suppression]->setDecimals(4);
	this->dv[Suppression]->setValue(g->suppression);
	form->addRow("Suppression", this->dv[Suppression]);
	this->iv[Cycles]->setValue(g->cycles);
	form->addRow("Cycles", this->iv[Cycles]);
	this->iv[Nodes]->setValue(g->nodes);
	form->addRow("Nodes", this->iv[Nodes]);
	this->iv[RayCount]->setValue(g->rayCount);
	form->addRow("Ray Count", this->iv[RayCount]);
	this->iv[RayLevels]->setValue(g->rayLevels);
	form->addRow("Ray Levels", this->iv[RayLevels]);
	this->iv[Depth]->setValue(g->depth);
	this->iv[Depth]->setRange(-10, 10);
	form->addRow("Volume Depth", this->iv[Depth]);
	setFormLayout(form);
	layout->addWidget(group);

	group = createGroup("Execute");
	form = createForm(group);
	this->startButton = new QPushButton("Erase && Generate", this);
	form->addRow(this->startButton);
	this->showVolumeButton = new QPushButton("Show Volume", this);
	form->addRow(this->showVolumeButton);
	this->hideVolumeButton = new QPushButton("Show Grid", this);
	form->addRow(this->hideVolumeButton);
	setFormLayout(form);
	layout->addWidget(group);

	layout->addStretch(1);

	for (int i = 0; i < DSize; i++)
		connect(this->dv[i],
			QOverload<double>::of(&DoubleSpinBox::valueChanged),
			this, &GeneratorEditor::change);
	for (int i = 0; i < ISize; i++)
		connect(this->iv[i],
			QOverload<int>::of(&SpinBox::valueChanged),
			this, &GeneratorEditor::change);

	connect(this->startButton, &QPushButton::clicked,
		this, &GeneratorEditor::start);

	connect(this->hideVolumeButton, &QPushButton::clicked, [&] () {
		this->editor->displayVolume(false);
		this->editor->change();
	});
	connect(this->showVolumeButton, &QPushButton::clicked, [&] () {
		this->editor->displayVolume(true);
		this->editor->change();
	});
}

void GeneratorEditor::change()
{
	Generator *g = &editor->getScene()->generator;
	g->primaryGrowthRate = this->dv[PrimaryRate]->value();
	g->secondaryGrowthRate = this->dv[SecondaryRate]->value();
	g->suppression = this->dv[Suppression]->value();
	g->rayCount = this->iv[RayCount]->value();
	g->rayLevels = this->iv[RayLevels]->value();
	g->cycles = this->iv[Cycles]->value();
	g->nodes = this->iv[Nodes]->value();
	g->depth = this->iv[Depth]->value();
}

void GeneratorEditor::start()
{
	this->startButton->setEnabled(false);
	this->editor->getScene()->updating = true;
	emit reset();
	emit generate();
}

void GeneratorEditor::end()
{
	this->startButton->setEnabled(true);
	this->editor->getScene()->updating = false;
	this->editor->displayVolume(true);
	this->editor->change();
}

GeneratorWorkload::GeneratorWorkload(Scene *scene) : scene(scene)
{

}

void GeneratorWorkload::generate()
{
	this->scene->generator.grow();
	emit done();
}

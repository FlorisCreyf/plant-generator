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
		this->dv[i]->setSingleStep(0.0001);
		this->dv[i]->setDecimals(4);
	}
	for (int i = 0; i < ISize; i++) {
		this->iv[i] = new SpinBox(this);
		this->iv[i]->setSingleStep(1);
		this->iv[i]->setRange(0, 1000);
	}

	QBoxLayout *layout = new QVBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(0);

	const int min = std::numeric_limits<int>::min();
	const int max = std::numeric_limits<int>::max();
	QGroupBox *group = createGroup("Properties");
	QFormLayout *form = createForm(group);
	form->addRow("Primary Growth", this->dv[PrimaryRate]);
	form->addRow("Secondary Growth", this->dv[SecondaryRate]);
	form->addRow("Suppression", this->dv[Suppression]);
	form->addRow("Suppression Falloff", this->dv[SuppressionFalloff]);
	form->addRow("Synthesis Rate", this->dv[SynthesisRate]);
	form->addRow("Synthesis Threshold", this->dv[SynthesisThreshold]);
	this->iv[Cycles]->setRange(1, 1000);
	form->addRow("Cycles", this->iv[Cycles]);
	this->iv[Nodes]->setRange(1, 1000);
	form->addRow("Nodes", this->iv[Nodes]);
	this->iv[Rays]->setRange(0, 100000);
	form->addRow("Rays", this->iv[Rays]);
	this->iv[Depth]->setRange(-10, 10);
	form->addRow("Volume Depth", this->iv[Depth]);
	this->dv[Optimization]->setRange(0.0f, 1.0f);
	form->addRow("Optimization", this->dv[Optimization]);
	this->iv[Seed]->setRange(min, max);
	form->addRow("Seed", this->iv[Seed]);
	setFormLayout(form);
	setValues();
	layout->addWidget(group);
	group = createGroup("Execute");
	form = createForm(group);
	this->startButton = new QPushButton("Erase && Generate", this);
	form->addRow(this->startButton);
	this->toggleVolumeButton = new QPushButton("Toggle Volume", this);
	form->addRow(this->toggleVolumeButton);
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
	connect(this->toggleVolumeButton, &QPushButton::clicked, [&] () {
		this->editor->displayVolume(!this->editor->showingVolume());
		this->editor->change();
	});
}

void GeneratorEditor::setValues()
{
	Generator *g = &this->editor->getScene()->generator;
	this->dv[PrimaryRate]->setValue(g->primaryGrowthRate);
	this->dv[SecondaryRate]->setValue(g->secondaryGrowthRate);
	this->dv[Suppression]->setValue(g->suppression);
	this->dv[SuppressionFalloff]->setValue(g->suppressionFalloff);
	this->dv[SynthesisRate]->setValue(g->synthesisRate);
	this->dv[SynthesisThreshold]->setValue(g->synthesisThreshold);
	this->dv[Optimization]->setValue(g->optimization);
	this->iv[Cycles]->setValue(g->cycles);
	this->iv[Nodes]->setValue(g->nodes);
	this->iv[Rays]->setValue(g->rays);
	this->iv[Depth]->setValue(g->depth);
	this->iv[Seed]->setValue(g->seed);
}

void GeneratorEditor::change()
{
	Generator *g = &editor->getScene()->generator;
	g->primaryGrowthRate = this->dv[PrimaryRate]->value();
	g->secondaryGrowthRate = this->dv[SecondaryRate]->value();
	g->suppression = this->dv[Suppression]->value();
	g->suppressionFalloff = this->dv[SuppressionFalloff]->value();
	g->synthesisThreshold = this->dv[SynthesisThreshold]->value();
	g->synthesisRate = this->dv[SynthesisRate]->value();
	g->optimization = this->dv[Optimization]->value();
	g->rays = this->iv[Rays]->value();
	g->cycles = this->iv[Cycles]->value();
	g->nodes = this->iv[Nodes]->value();
	g->depth = this->iv[Depth]->value();
	g->seed = this->iv[Seed]->value();
}

void GeneratorEditor::start()
{
	this->startButton->setEnabled(false);
	emit reset();
	this->editor->getScene()->updating = true;
	emit generate();
}

void GeneratorEditor::end()
{
	this->startButton->setEnabled(true);
	this->editor->getScene()->updating = false;
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

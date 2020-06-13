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

#include "generator_editor.h"
#include "../commands/generate.h"
#include <limits>

GeneratorEditor::GeneratorEditor(Editor *editor, QWidget *parent) :
	Form(editor, parent)
{
	this->editor = editor;
	createInterface();
}

QSize GeneratorEditor::sizeHint() const
{
	return QSize(350, 200);
}

void GeneratorEditor::createInterface()
{
	QFormLayout *form = new QFormLayout(this);
	form->setSizeConstraint(QLayout::SetMinimumSize);
	form->setSpacing(0);
	form->setMargin(0);
	form->setSpacing(2);
	form->setMargin(5);

	this->seed = new QSpinBox(this);
	this->seed->setRange(
		std::numeric_limits<int>::min(),
		std::numeric_limits<int>::max());
	this->seed->setSingleStep(1);
	form->addRow(tr("Seed"), this->seed);
	this->stemDensity = new QDoubleSpinBox(this);
	this->stemDensity->setSingleStep(0.1);
	form->addRow(tr("Stem Density"), this->stemDensity);
	this->stemStart = new QDoubleSpinBox(this);
	this->stemStart->setSingleStep(0.1);
	form->addRow(tr("Stem Start"), this->stemStart);
	this->leafDensity = new QDoubleSpinBox(this);
	this->leafDensity->setSingleStep(0.1);
	form->addRow(tr("Leaf Density"), this->leafDensity);
	this->leafStart = new QDoubleSpinBox(this);
	this->leafStart->setSingleStep(0.1);
	form->addRow(tr("Leaf Start"), this->leafStart);
	this->createButton = new QPushButton(tr("Generate"), this);
	this->createButton->setFixedHeight(22);
	form->addRow(this->createButton);

	connect(this->createButton, SIGNAL(clicked()),this, SLOT(submit()));
}

void GeneratorEditor::submit()
{
	Generate *generate = new Generate(this->editor->getSelection());
	pg::PseudoGenerator gen(this->editor->getPlant());
	gen.setLeafCount(this->leafDensity->value(), this->leafStart->value());
	gen.setStemCount(this->stemDensity->value(), this->stemStart->value());
	gen.setSeed(this->seed->value());
	generate->setGenerator(std::move(gen));
	generate->execute();
	this->editor->add(generate);
	this->editor->change();
}

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

#include "wind_editor.h"
#include "form.h"

WindEditor::WindEditor(Editor *editor, QWidget *parent) :
	QWidget(parent), editor(editor)
{
	createInterface();
}

inline QGroupBox *createGroup(const char *name)
{
	QGroupBox *group = new QGroupBox(name);
	group->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	return group;
}

void WindEditor::createInterface()
{
	this->group = createGroup("Wind");
	QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
	layout->setMargin(0);
	layout->setSpacing(0);
	layout->addWidget(this->group);
	QFormLayout *form = new QFormLayout(this->group);
	form->setSpacing(UI_FORM_SPACING);
	form->setMargin(UI_FORM_MARGIN);

	for (int i = 0; i < DSize; i++) {
		this->dv[i] = new DoubleSpinBox(this);
		this->dv[i]->setDecimals(3);
		this->dv[i]->setSingleStep(0.01);
		this->dv[i]->setRange(
			std::numeric_limits<float>::lowest(),
			std::numeric_limits<float>::max());
		connect(this->dv[i],
			QOverload<double>::of(&QDoubleSpinBox::valueChanged),
			this, &WindEditor::change);
	}
	for (int i = 0; i < ISize; i++) {
		this->iv[i] = new SpinBox(this);
		connect(this->iv[i],
			QOverload<int>::of(&QSpinBox::valueChanged),
			this, &WindEditor::change);
	}

	form->addRow("Seed", this->iv[Seed]);
	this->iv[Seed]->setRange(
		std::numeric_limits<int>::min(),
		std::numeric_limits<int>::max());
	this->iv[Seed]->setSingleStep(1);
	form->addRow("Direction.X", this->dv[DirectionX]);
	form->addRow("Direction.Y", this->dv[DirectionY]);
	form->addRow("Direction.Z", this->dv[DirectionZ]);
	form->addRow("Time Step", this->iv[TimeStep]);
	this->iv[TimeStep]->setValue(30);
	form->addRow("Frames", this->iv[FrameCount]);
	this->iv[FrameCount]->setValue(21);

	setFormLayout(form);
	layout->addWidget(group);
}

void WindEditor::change()
{
	pg::Wind *wind = &this->editor->getScene()->wind;
	pg::Vec3 direction;
	direction.x = this->dv[DirectionX]->value();
	direction.y = this->dv[DirectionY]->value();
	direction.z = this->dv[DirectionZ]->value();
	float speed = pg::magnitude(direction);
	if (speed > 0.0f)
		direction /= speed;

	wind->setSeed(this->iv[Seed]->value());
	wind->setDirection(direction);
	wind->setSpeed(speed);
	wind->setFrameCount(this->iv[FrameCount]->value());
	wind->setTimeStep(this->iv[TimeStep]->value());
	this->editor->changeWind();
}

void WindEditor::setFields()
{
	blockSignals(true);
	const pg::Wind *wind = &this->editor->getScene()->wind;
	this->dv[DirectionX]->setValue(wind->getDirection().x);
	this->dv[DirectionY]->setValue(wind->getDirection().y);
	this->dv[DirectionZ]->setValue(wind->getDirection().z);
	this->iv[Seed]->setValue(wind->getSeed());
	this->iv[FrameCount]->setValue(wind->getFrameCount());
	this->iv[TimeStep]->setValue(wind->getTimeStep());
	blockSignals(false);
}

void WindEditor::blockSignals(bool block)
{
	for (int i = 0; i < DSize; i++)
		this->dv[i]->blockSignals(block);
	for (int i = 0; i < ISize; i++)
		this->iv[i]->blockSignals(block);
}

QSize WindEditor::sizeHint() const
{
	return QSize(UI_WIDGET_WIDTH, UI_WIDGET_HEIGHT);
}

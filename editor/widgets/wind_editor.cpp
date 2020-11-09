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
#include "definitions.h"

WindEditor::WindEditor(Editor *editor, QWidget *parent) : Form(parent)
{
	this->editor = editor;
	createInterface();
}

void WindEditor::createInterface()
{
	this->group = new QGroupBox("Wind", this);
	this->group->setSizePolicy(
		QSizePolicy::Expanding, QSizePolicy::Minimum);

	QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
	layout->setMargin(0);
	layout->setSpacing(0);
	layout->addWidget(this->group);

	QFormLayout *form = new QFormLayout(this->group);
	form->setSpacing(UI_FORM_SPACING);
	form->setMargin(UI_FORM_MARGIN);

	this->seedValue = new QSpinBox(this);
	this->seedValue->setRange(
		std::numeric_limits<int>::min(),
		std::numeric_limits<int>::max());
	this->seedValue->setSingleStep(1);
	form->addRow("Seed", this->seedValue);

	this->directionXValue = new QDoubleSpinBox(this);
	this->directionXValue->setSingleStep(0.01);
	this->directionXValue->setDecimals(3);
	this->directionXValue->setRange(
		std::numeric_limits<float>::lowest(),
		std::numeric_limits<float>::max());
	form->addRow("Direction.X", this->directionXValue);

	this->directionYValue = new QDoubleSpinBox(this);
	this->directionYValue->setSingleStep(0.01);
	this->directionYValue->setDecimals(3);
	this->directionYValue->setRange(
		std::numeric_limits<float>::lowest(),
		std::numeric_limits<float>::max());
	form->addRow("Direction.Y", this->directionYValue);

	this->directionZValue = new QDoubleSpinBox(this);
	this->directionZValue->setSingleStep(0.01);
	this->directionZValue->setDecimals(3);
	this->directionZValue->setValue(1.0);
	this->directionZValue->setRange(
		std::numeric_limits<float>::lowest(),
		std::numeric_limits<float>::max());
	form->addRow("Direction.Z", this->directionZValue);

	this->timeStepValue = new QSpinBox(this);
	this->timeStepValue->setValue(30);
	form->addRow("Time Step", this->timeStepValue);

	this->frameCountValue = new QSpinBox(this);
	this->frameCountValue->setValue(21);
	form->addRow("Frames", this->frameCountValue);

	setValueWidths(form);

	connect(this->seedValue, SIGNAL(valueChanged(int)),
		this, SLOT(change()));
	connect(this->directionXValue, SIGNAL(valueChanged(double)),
		this, SLOT(change()));
	connect(this->directionYValue, SIGNAL(valueChanged(double)),
		this, SLOT(change()));
	connect(this->directionZValue, SIGNAL(valueChanged(double)),
		this, SLOT(change()));
	connect(this->timeStepValue, SIGNAL(valueChanged(int)),
		this, SLOT(change()));
	connect(this->frameCountValue, SIGNAL(valueChanged(int)),
		this, SLOT(change()));
}

void WindEditor::change()
{
	pg::Wind *wind = &this->editor->getScene()->wind;
	pg::Vec3 direction;
	direction.x = this->directionXValue->value();
	direction.y = this->directionYValue->value();
	direction.z = this->directionZValue->value();
	float speed = pg::magnitude(direction);
	if (speed > 0.0f)
		direction /= speed;

	wind->setSeed(this->seedValue->value());
	wind->setDirection(direction);
	wind->setSpeed(speed);
	wind->setFrameCount(this->frameCountValue->value());
	wind->setTimeStep(this->timeStepValue->value());
	this->editor->changeWind();
}

QSize WindEditor::sizeHint() const
{
	return QSize(UI_WIDGET_WIDTH, UI_WIDGET_HEIGHT);
}

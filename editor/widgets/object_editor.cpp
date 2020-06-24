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

#include "object_editor.h"
#include "item_delegate.h"
#include "definitions.h"

ObjectEditor::ObjectEditor(QWidget *parent) : QWidget(parent)
{
	this->layout = new QVBoxLayout(this);
	this->layout->setSizeConstraint(QLayout::SetMinimumSize);
	this->layout->setSpacing(0);
	this->layout->setMargin(0);

	QHBoxLayout *topRow = new QHBoxLayout();
	topRow->setSizeConstraint(QLayout::SetMinimumSize);
	topRow->setSpacing(0);
	topRow->setMargin(0);
	initTopRow(topRow);
	this->layout->addLayout(topRow);
}

QSize ObjectEditor::sizeHint() const
{
	return QSize(UI_WIDGET_WIDTH, UI_WIDGET_WIDTH);
}

void ObjectEditor::initTopRow(QHBoxLayout *topRow)
{
	this->addButton = new QPushButton("+", this);
	this->addButton->setFixedHeight(UI_FIELD_HEIGHT);
	this->addButton->setFixedWidth(UI_FIELD_HEIGHT);

	this->removeButton = new QPushButton("-", this);
	this->removeButton->setFixedHeight(UI_FIELD_HEIGHT);
	this->removeButton->setFixedWidth(UI_FIELD_HEIGHT);

	this->selectionBox = new QComboBox(this);
	this->selectionBox->setEditable(true);
	this->selectionBox->setInsertPolicy(QComboBox::InsertAtCurrent);
	this->selectionBox->setItemDelegate(new ItemDelegate());

	topRow->addWidget(this->selectionBox);
	topRow->addWidget(this->removeButton);
	topRow->addWidget(this->addButton);
	topRow->setAlignment(Qt::AlignTop);

	connect(this->selectionBox->lineEdit(), SIGNAL(editingFinished()),
		this, SLOT(rename()));
	connect(this->addButton, SIGNAL(clicked()),
		this, SLOT(add()));
	connect(this->removeButton, SIGNAL(clicked()),
		this, SLOT(remove()));
	connect(this->selectionBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(select()));
}

/* Plant Genererator
 * Copyright (C) 2020  Floris Creyf
 *
 * Plant Genererator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Plant Genererator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "form.h"

Form::Form(Editor *editor, QWidget *parent) : QWidget(parent)
{
	this->editor = editor;
	this->changing = false;
}

void Form::setValueWidths(QFormLayout *layout)
{
	layout->setFormAlignment(Qt::AlignRight | Qt::AlignTop);
	layout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
	layout->setLabelAlignment(Qt::AlignRight);
	for(int i = 0; i < layout->rowCount(); i++) {
		QLayoutItem *item = layout->itemAt(i, QFormLayout::FieldRole);
		if (item && item->widget())
			item->widget()->setFixedWidth(250);
	}
}

void Form::indicateDifferences(QWidget *widget)
{
	widget->setStyleSheet("font-weight:bold;");
}

void Form::indicateSimilarities(QWidget *widget)
{
	widget->setStyleSheet("");
}

void Form::beginChanging()
{
	if (!this->changing) {
		this->saveStem = new SaveStem(this->editor->getSelection());
		this->saveStem->execute();
	}
	this->changing = true;
}

void Form::finishChanging()
{
	if (this->changing && !this->saveStem->isSameAsCurrent()) {
		this->saveStem->setNewSelection();
		this->editor->add(*this->saveStem);
	}
	this->changing = false;
}

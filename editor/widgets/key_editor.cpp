/* Plant Genererator
 * Copyright (C) 2019  Floris Creyf
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

#include "key_editor.h"

KeyEditor::KeyEditor(KeyMap *keymap, QWidget *parent) : QWidget(parent)
{
	form = new QFormLayout(this);
	form->setSpacing(2);
	form->setMargin(10);
	form->setLabelAlignment(Qt::AlignRight);

	this->keymap = keymap;

	auto bindings = keymap->getBindings();
	for (auto binding : bindings) {
		QPushButton *button;
		initField(&button, binding.second);
		button->setText(keymap->toString(binding.first));
	}

	setValueWidths(form);
}

QSize KeyEditor::sizeHint() const
{
	return QSize(350, 200);
}

void KeyEditor::initField(QPushButton **widget, QString name)
{
	QHBoxLayout *layout = new QHBoxLayout();
	*widget = new QPushButton();
	(*widget)->setFixedHeight(22);
	(*widget)->setFixedWidth(250);
	(*widget)->setEnabled(false);
	layout->addWidget(*widget);
	form->addRow(name, layout);
}

void KeyEditor::setValueWidths(QFormLayout *layout)
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

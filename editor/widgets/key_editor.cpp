/* Plant Generator
 * Copyright (C) 2019  Floris Creyf
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

#include "key_editor.h"
#include "definitions.h"

KeyEditor::KeyEditor(KeyMap *keymap, QWidget *parent) : QWidget(parent)
{
	form = new QFormLayout(this);
	form->setSpacing(UI_FORM_SPACING);
	form->setMargin(UI_FORM_MARGIN);
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
	return QSize(UI_WIDGET_WIDTH, UI_WIDGET_WIDTH);
}

void KeyEditor::initField(QPushButton **widget, QString name)
{
	QHBoxLayout *layout = new QHBoxLayout();
	*widget = new QPushButton();
	(*widget)->setFixedHeight(UI_FIELD_HEIGHT);
	(*widget)->setFixedWidth(UI_FIELD_WIDTH);
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
			item->widget()->setFixedWidth(UI_FIELD_WIDTH);
	}
}

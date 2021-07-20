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

#include "form.h"

void setFormLayout(QFormLayout *layout)
{
	layout->setFormAlignment(Qt::AlignRight | Qt::AlignTop);
	layout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
	layout->setLabelAlignment(Qt::AlignRight);
	for (int i = 0; i < layout->rowCount(); i++) {
		QLayoutItem *item = layout->itemAt(i, QFormLayout::FieldRole);
		QWidget *label = layout->labelForField(item->widget());
		if (item && item->widget()) {
			if (label)
				item->widget()->setFixedWidth(UI_FIELD_WIDTH);
			item->widget()->setFixedHeight(UI_FIELD_HEIGHT);
		}
	}
}

void indicateDifferences(QWidget *widget)
{
	widget->setStyleSheet("font-weight:bold;");
}

void indicateSimilarities(QWidget *widget)
{
	widget->setStyleSheet("");
}

QGroupBox *createGroup(const char *name)
{
	QGroupBox *group = new QGroupBox(name);
	group->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	return group;
}

QFormLayout *createForm(QWidget *widget)
{
	QFormLayout *form = new QFormLayout(widget);
	form->setSpacing(UI_FORM_SPACING);
	form->setMargin(UI_FORM_MARGIN);
	return form;
}

QFormLayout *createForm(QBoxLayout *layout)
{
	QFormLayout *form = new QFormLayout();
	form->setSpacing(UI_FORM_SPACING);
	form->setMargin(UI_FORM_MARGIN);
	layout->addLayout(form);
	return form;
}

QWidget *createCBField(ComboBox *cb, QPushButton *b1, QPushButton *b2)
{
	QHBoxLayout *row = new QHBoxLayout();
	QWidget *field = new QWidget();
	field->setLayout(row);
	row->setSpacing(0);
	row->setMargin(0);
	b1->setFixedHeight(UI_FIELD_HEIGHT);
	b1->setFixedWidth(UI_FIELD_HEIGHT);
	b2->setFixedHeight(UI_FIELD_HEIGHT);
	b2->setFixedWidth(UI_FIELD_HEIGHT);
	cb->setEditable(true);
	cb->setInsertPolicy(ComboBox::InsertAtCurrent);
	row->addWidget(cb);
	row->addWidget(b1);
	row->addWidget(b2);
	row->setAlignment(Qt::AlignTop);
	return field;
}

QWidget *createLEField(QLineEdit *le, QPushButton *b1, QPushButton *b2)
{
	QHBoxLayout *row = new QHBoxLayout();
	QWidget *field = new QWidget();
	field->setLayout(row);
	le->setFixedHeight(UI_FIELD_HEIGHT);
	le->setReadOnly(true);
	b1->setFixedWidth(UI_FIELD_HEIGHT);
	b1->setFixedHeight(UI_FIELD_HEIGHT);
	b2->setFixedWidth(UI_FIELD_HEIGHT);
	b2->setFixedHeight(UI_FIELD_HEIGHT);
	row->setSpacing(0);
	row->setMargin(0);
	row->addWidget(le);
	row->addWidget(b1);
	row->addWidget(b2);
	return field;
}

QString createUniqueName(const char *prefix, QComboBox *names)
{
	for (int i = 1; true; i++) {
		QString name = prefix + QString::number(i);
		if (names->findText(name) == -1)
			return name;
	}
}

void clearFocusIfDescendant(QWidget *widget)
{
	QWidget *focusWidget = QApplication::focusWidget();
	while (focusWidget && focusWidget != widget)
		focusWidget = focusWidget->parentWidget();
	if (focusWidget == widget)
		QApplication::focusWidget()->clearFocus();
}

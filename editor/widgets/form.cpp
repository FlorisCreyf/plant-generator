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
#include "definitions.h"

void setValueWidths(QFormLayout *layout)
{
	layout->setFormAlignment(Qt::AlignRight | Qt::AlignTop);
	layout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
	layout->setLabelAlignment(Qt::AlignRight);
	for(int i = 0; i < layout->rowCount(); i++) {
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

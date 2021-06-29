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

#include "widgets.h"

ComboBox::ComboBox(QWidget *parent) : QComboBox(parent)
{
	setFocusPolicy(Qt::StrongFocus);
	this->setItemDelegate(new SID());
}

void ComboBox::wheelEvent(QWheelEvent *event)
{
	if (!hasFocus())
		event->ignore();
	else
		QComboBox::wheelEvent(event);
}

SpinBox::SpinBox(QWidget *parent) : QSpinBox(parent)
{
	setFocusPolicy(Qt::StrongFocus);
}

void SpinBox::wheelEvent(QWheelEvent *event)
{
	if (!hasFocus())
		event->ignore();
	else
		QSpinBox::wheelEvent(event);
}

DoubleSpinBox::DoubleSpinBox(QWidget *parent) : QDoubleSpinBox(parent)
{
	setFocusPolicy(Qt::StrongFocus);
}

void DoubleSpinBox::wheelEvent(QWheelEvent *event)
{
	if (!hasFocus())
		event->ignore();
	else
		QDoubleSpinBox::wheelEvent(event);
}

QSize SID::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
	return QSize(1, 22);
}

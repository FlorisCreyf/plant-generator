/* Plant Generator
 * Copyright (C) 2018  Floris Creyf
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

#ifndef WIDGETS_H
#define WIDGETS_H

#include <QtWidgets>

class ComboBox : public QComboBox {
	Q_OBJECT

public:
	ComboBox(QWidget *parent = 0);

protected:
	virtual void wheelEvent(QWheelEvent *event);

};

class SpinBox : public QSpinBox {
	Q_OBJECT

public:
	SpinBox(QWidget *parent = 0);

protected:
	virtual void wheelEvent(QWheelEvent *event);

};

class DoubleSpinBox : public QDoubleSpinBox {
	Q_OBJECT

public:
	DoubleSpinBox(QWidget *parent = 0);

protected:
	virtual void wheelEvent(QWheelEvent *event);
};

class ItemDelegate : public QStyledItemDelegate {
	Q_OBJECT

public:
	QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const;
};

#endif

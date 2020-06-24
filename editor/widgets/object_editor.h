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

#ifndef OBJECT_EDITOR_H
#define OBJECT_EDITOR_H

#include <QtWidgets>

class ObjectEditor : public QWidget {
	Q_OBJECT

	void initTopRow(QHBoxLayout *topRow);

protected:
	QVBoxLayout *layout;
	QComboBox *selectionBox;
	QPushButton *addButton;
	QPushButton *removeButton;

public:
	ObjectEditor(QWidget *parent);
	QSize sizeHint() const;

public slots:
	virtual void add() = 0;
	virtual void rename() = 0;
	virtual void remove() = 0;
	virtual void select() = 0;
};

#endif /* OBJECT_EDITOR_H */

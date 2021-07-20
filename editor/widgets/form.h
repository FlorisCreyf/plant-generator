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

#define UI_FIELD_HEIGHT 20
#define UI_FIELD_WIDTH 200
#define UI_WIDGET_WIDTH 350
#define UI_WIDGET_HEIGHT 200
#define UI_FORM_MARGIN 5
#define UI_FORM_SPACING 2

#ifndef FORM_H
#define FORM_H

#include <QtWidgets>
#include "widgets.h"

void setFormLayout(QFormLayout *layout);
void indicateDifferences(QWidget *widget);
void indicateSimilarities(QWidget *widget);
QGroupBox *createGroup(const char *name);
QFormLayout *createForm(QWidget *widget);
QFormLayout *createForm(QBoxLayout *layout);
QWidget *createCBField(ComboBox *cb, QPushButton *b1, QPushButton *b2);
QWidget *createLEField(QLineEdit *le, QPushButton *b1, QPushButton *b2);
QString createUniqueName(const char *prefix, QComboBox *names);
void clearFocusIfDescendant(QWidget *widget);

#endif

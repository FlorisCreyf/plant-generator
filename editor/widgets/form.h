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

#ifndef FORM_H
#define FORM_H

#include "editor.h"
#include "editor/commands/save_stem.h"
#include <QtWidgets>

class Form : public QWidget {
	Q_OBJECT
public:
	Form(QWidget *parent);	

protected:
	void setValueWidths(QFormLayout *layout);
	void indicateDifferences(QWidget *widget);
	void indicateSimilarities(QWidget *widget);

private:
	Editor *editor;
};

#endif

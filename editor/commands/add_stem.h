/* Plant Genererator
 * Copyright (C) 2018  Floris Creyf
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

#ifndef ADD_STEM_H
#define ADD_STEM_H

#include "command.h"
#include "../stem_selection.h"
#include "../geometry/translation_axes.h"

class AddStem : public Command {
	StemSelection *selection;
	StemSelection prevSelection;
	pg::Stem *stem;
	bool undone;

	void create();

public:
	AddStem(StemSelection *selection);
	~AddStem();
	void execute();
	void undo();
	AddStem *clone();
};

#endif /* ADD_STEM_H */

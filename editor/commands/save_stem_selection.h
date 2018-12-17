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

#ifndef SAVE_STEM_SELECTION_H
#define SAVE_STEM_SELECTION_H

#include "command.h"
#include "../stem_selection.h"

class SaveStemSelection : public Command {
	StemSelection *selection;
	StemSelection after;
	StemSelection before;

public:
	SaveStemSelection(StemSelection *selection);
	bool hasChanged() const;
	void setBefore();
	void setAfter();
	void execute();
	void undo();
	SaveStemSelection *clone();
};

#endif /* SAVE_STEM_SELECTION_H */

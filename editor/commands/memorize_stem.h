/* Plant Genererator
 * Copyright (C) 2018  Floris Creyf
 *
 * Plant Genererator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.,
 *
 * Plant Genererator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MEMORIZE_STEM_H
#define MEMORIZE_STEM_H

#include "command.h"
#include "../stem_selection.h"
#include <map>

class MemorizeStem : public Command {
	StemSelection *selection;
	std::map<pg::Stem *, pg::Stem> stems;
	bool undone;
	
	void swap();
	
public:
	MemorizeStem(StemSelection *selection);
	bool isSameAsCurrent();
	void execute();
	void undo();
	MemorizeStem *clone();
};

#endif /* MEMORIZE_STEM_H */


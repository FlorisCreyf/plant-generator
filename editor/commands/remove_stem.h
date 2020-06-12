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

#ifndef REMOVE_STEM_H
#define REMOVE_STEM_H

#include "command.h"
#include "editor/selection.h"
#include <memory>
#include <utility>

class RemoveStem : public Command {
	struct LeafState {
		size_t index;
		pg::Leaf leaf;
		pg::Stem *stem;
	};
	Selection *selection;
	Selection prevSelection;
	std::vector<LeafState> leaves;
	std::vector<std::pair<pg::Stem *, pg::Spline>> splines;
	std::vector<pg::Stem *> stems;

	void removeLeaves();
	void removeStems();

public:
	RemoveStem(Selection *selection);
	~RemoveStem();
	void execute();
	void undo();
};

#endif /* REMOVE_STEM_H */

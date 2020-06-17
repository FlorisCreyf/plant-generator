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

#ifndef GENERATE_H
#define GENERATE_H

#include "command.h"
#include "remove_stem.h"
#include "editor/selection.h"
#include "plant_generator/pseudo_generator.h"

class Generate : public Command {
	Selection prevSelection;
	Selection removals;
	Selection *selection;
	RemoveStem remove;
	std::vector<pg::Derivation> derivations;
	pg::PseudoGenerator gen;

	void createRemovalSelection(Selection *, Selection *);
	void removeAdditions();

public:
	Generate(Selection *selection);
	void setGenerator(pg::PseudoGenerator gen);
	void execute();
	void undo();
	void redo();
};

#endif /* GENERATE_H */

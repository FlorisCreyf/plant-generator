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

#ifndef HISTORY_H
#define HISTORY_H

#include "selection_state.h"
#include "editor/commands/command.h"
#include <vector>
#include <memory>

class History {
	struct Memento {
		Memento(Command &command, const SelectionState &selection) :
			command(command.clone()),
			selection(selection.clone())
		{
		
		}
		std::shared_ptr<Command> command;
		std::shared_ptr<SelectionState> selection;
	};
	std::vector<std::shared_ptr<SelectionState>> pastSelections;
	std::vector<std::shared_ptr<SelectionState>> futureSelections;
	std::shared_ptr<SelectionState> currentSelection;
	std::vector<Memento> past;
	std::vector<Memento> future;
	
public:
	History();
	void add(Command &command, const SelectionState &selection);
	void add(const SelectionState &selection);
	void undo();
	void redo();
	void clear();
};

#endif /* HISTORY_H */

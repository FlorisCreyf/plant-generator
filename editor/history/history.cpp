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

#include "history.h"

using std::vector;

History::History() : currentSelection(nullptr)
{

}

void History::add(Command &command, const SelectionState &selection)
{
	Memento memento(command, selection);
	past.push_back(std::move(memento));
	future.clear();
	currentSelection = memento.selection;
	pastSelections.clear();
	futureSelections.clear();
	add(selection);
}

void History::add(const SelectionState &selection)
{
	if (currentSelection && currentSelection->hasChanged()) {
		pastSelections.push_back(currentSelection);
		currentSelection.reset(selection.clone());
		futureSelections.clear();
		future.clear();
	} else if (!currentSelection) {
		currentSelection.reset(selection.clone());
		future.clear();
	}
}

void History::undo()
{
	if (!pastSelections.empty() && future.empty()) {
		futureSelections.push_back(currentSelection);
		currentSelection = pastSelections.back();
		currentSelection->replaceSelection();
		pastSelections.pop_back();
	} else if (!past.empty()) {
		past.back().selection->replaceSelection();
		past.back().command->undo();
		past.back().selection->swapSelection();
		future.push_back(past.back());
		past.pop_back();
	}
}

void History::redo()
{
	if (!futureSelections.empty() && future.empty()) {
		pastSelections.push_back(currentSelection);
		currentSelection = futureSelections.back();
		currentSelection->replaceSelection();
		futureSelections.pop_back();
	} else if (!future.empty()) {
		future.back().selection->replaceSelection();
		future.back().command->execute();
		future.back().selection->swapSelection();
		past.push_back(future.back());
		future.pop_back();
	}
}

void History::clear()
{
	future.clear();
	past.clear();
	futureSelections.clear();
	pastSelections.clear();
	currentSelection = nullptr;
}

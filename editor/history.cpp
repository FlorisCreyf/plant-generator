/* Plant Generator
 * Copyright (C) 2018  Floris Creyf
 *
 * Plant Generator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.,
 *
 * Plant Generator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "history.h"

using std::vector;

void History::add(Command *command)
{
	future.clear();

	/* The ordering of signals and events might cause commands to be out
	of order. For example, a click event changes the selection and then
	an editingFinished signal adds an earlier command. */
	if (!past.empty() && command->getTime() < past.back()->getTime()) {
		auto it = past.end();
		std::unique_ptr<Command> cmd(command);
		past.insert(--it, std::move(cmd));
	} else {
		std::unique_ptr<Command> cmd(command);
		past.push_back(std::move(cmd));
	}
}

void History::undo()
{
	if (!past.empty()) {
		past.back()->undo();
		future.push_back(std::move(past.back()));
		past.pop_back();
	}
}

void History::redo()
{
	if (!future.empty()) {
		future.back()->redo();
		past.push_back(std::move(future.back()));
		future.pop_back();
	}
}

void History::clear()
{
	future.clear();
	past.clear();
}

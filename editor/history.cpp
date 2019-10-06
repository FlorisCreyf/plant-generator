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

void History::add(Command *command)
{
	future.clear();
	std::shared_ptr<Command> cmd(command);
	past.push_back(std::move(cmd));
}

void History::undo()
{
	if (!past.empty()) {
		past.back()->undo();
		future.push_back(past.back());
		past.pop_back();
	}
}

void History::redo()
{
	if (!future.empty()) {
		future.back()->redo();
		past.push_back(future.back());
		future.pop_back();
	}
}

void History::clear()
{
	future.clear();
	past.clear();
}

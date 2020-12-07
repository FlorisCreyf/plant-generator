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

#ifndef HISTORY_H
#define HISTORY_H

#include "commands/command.h"
#include <vector>
#include <memory>

class History {
	std::vector<std::unique_ptr<Command>> past;
	std::vector<std::unique_ptr<Command>> future;
	unsigned limit;

public:
	History();
	void add(Command *command);
	void undo();
	void redo();
	void clear();
	void setLimit(unsigned limit);
};

#endif

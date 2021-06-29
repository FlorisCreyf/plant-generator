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

History::History() : limit(20)
{

}

void History::add(Command *command)
{
	this->future.clear();
	if (this->past.size() >= limit)
		this->past.erase(this->past.begin());

	/* The ordering of signals and events might cause commands to be out
	of order. For example, a click event changes the selection and then
	an editingFinished signal adds an earlier command. */
	bool empty = this->past.empty();
	if (!empty && command->getTime() < this->past.back()->getTime()) {
		auto it = this->past.end();
		std::unique_ptr<Command> cmd(command);
		this->past.insert(--it, std::move(cmd));
	} else {
		std::unique_ptr<Command> cmd(command);
		this->past.push_back(std::move(cmd));
	}
}

void History::undo()
{
	if (!this->past.empty()) {
		this->past.back()->undo();
		this->future.push_back(std::move(this->past.back()));
		this->past.pop_back();
	}
}

void History::redo()
{
	if (!this->future.empty()) {
		this->future.back()->redo();
		this->past.push_back(std::move(this->future.back()));
		this->future.pop_back();
	}
}

const Command *History::peak()
{
	if (this->past.size() > 0)
		return this->past.back().get();
	else
		return nullptr;
}

void History::clear()
{
	this->future.clear();
	this->past.clear();
}

void History::setLimit(unsigned limit)
{
	this->limit = limit;
}

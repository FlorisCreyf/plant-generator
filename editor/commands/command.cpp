/* Plant Genererator
 * Copyright (C) 2019  Floris Creyf
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

#include "command.h"

Command::Command()
{

}

Command::Command(const Command &original)
{
	this->done = original.done;
}

bool Command::onMouseMove(QMouseEvent *)
{
	return false;
}

bool Command::onMousePress(QMouseEvent *)
{
	return false;
}

bool Command::onMouseRelease(QMouseEvent *)
{
	return false;
}

bool Command::onKeyPress(QKeyEvent *)
{
	return false;
}

void Command::redo()
{
	execute();
}

bool Command::isDone()
{
	return done;
}
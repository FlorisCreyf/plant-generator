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

#ifndef COMMAND_H
#define COMMAND_H

#include <QMouseEvent>
#include <QKeyEvent>
#include <ctime>

class Command {
	time_t timer;

protected:
	bool done = false;

public:
	Command();
	virtual void execute() = 0;
	virtual void undo() = 0;
	virtual void redo();
	virtual bool isDone();
	virtual bool onMouseMove(QMouseEvent *);
	virtual bool onMousePress(QMouseEvent *);
	virtual bool onMouseRelease(QMouseEvent *);
	virtual bool onKeyPress(QKeyEvent *);
	time_t getTime() const;
};

#endif /* COMMAND_H */

/* Plant Genererator
 * Copyright (C) 2018  Floris Creyf
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

#ifndef COMMAND_H
#define COMMAND_H

#include <QMouseEvent>
#include <QKeyEvent>

class Command {
	/** Commands should not be copied if it makes ownership of stems and
	other resources unclear. */
	Command(const Command &);

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
};

#endif /* COMMAND_H */

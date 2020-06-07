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

#ifndef SAVE_SELECTION_H
#define SAVE_SELECTION_H

#include "command.h"
#include "editor/selection.h"

class SaveSelection : public Command {
	Selection *selection;
	Selection after;
	Selection before;

public:
	SaveSelection(Selection *selection);
	bool hasChanged() const;
	void setBefore();
	void setAfter();
	void execute();
	void undo();
};

#endif /* SAVE_SELECTION_H */

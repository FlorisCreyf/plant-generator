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

#ifndef SAVE_POINT_SELECTION_H
#define SAVE_POINT_SELECTION_H

#include "command.h"
#include "../point_selection.h"

class SavePointSelection : public Command {
	PointSelection *selection;
	PointSelection after;
	PointSelection before;

public:
	SavePointSelection(PointSelection *selection);
	bool hasChanged() const;
	void setBefore();
	void setAfter();
	void execute();
	void undo();
	SavePointSelection *clone();
};

#endif /* SAVE_POINT_SELECTION_H */

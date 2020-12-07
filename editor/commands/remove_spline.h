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

#ifndef REMOVE_SPLINE_H
#define REMOVE_SPLINE_H

#include "command.h"
#include "editor/point_selection.h"
#include "plant_generator/spline.h"

class RemoveSpline : public Command {
	PointSelection *selection;
	PointSelection prevSelection;
	pg::Spline *spline;
	pg::Spline prevSpline;
	bool clearable;

public:
	RemoveSpline(PointSelection *selection, pg::Spline *spline);
	RemoveSpline(const RemoveSpline &original);
	RemoveSpline &operator=(const RemoveSpline &original);
	void setClearable(bool clearable);
	void execute();
	void undo();
};

#endif

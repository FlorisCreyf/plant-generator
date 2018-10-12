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

#ifndef EXTRUDE_SPLINE_H
#define EXTRUDE_SPLINE_H

#include "command.h"
#include "../point_selection.h"
#include "plant_generator/spline.h"
#include <memory>

class ExtrudeSpline : public Command {
	PointSelection *selection;
	std::shared_ptr<PointSelection> prevSelection;
	std::shared_ptr<PointSelection> newSelection;
	pg::Spline *spline;
	pg::Spline prevSpline;

public:
	ExtrudeSpline(PointSelection *selection, pg::Spline *spline);
	void execute();
	void undo();
	ExtrudeSpline *clone();
};

#endif /* EXTRUDE_SPLINE_H */

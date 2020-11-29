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

#ifndef EXTRUDE_SPLINE_H
#define EXTRUDE_SPLINE_H

#include "command.h"
#include "move_spline.h"
#include "editor/camera.h"
#include "editor/geometry/translation_axes.h"
#include "editor/point_selection.h"
#include "plant_generator/spline.h"
#include <memory>

class ExtrudeSpline : public Command {
	PointSelection *selection;
	PointSelection prevSelection;
	PointSelection newSelection;
	pg::Spline *spline;
	pg::Spline prevSpline;
	MoveSpline moveSpline;

public:
	ExtrudeSpline(PointSelection *selection, pg::Spline *spline,
		TranslationAxes *axes, const Camera *camera);
	void setClickOffset(int x, int y);
	void execute();
	void undo();
	void redo();
	bool onMouseMove(QMouseEvent *event);
	bool onMousePress(QMouseEvent *event);
	bool onMouseRelease(QMouseEvent *event);
};

#endif

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

#ifndef MOVE_SPLINE_H
#define MOVE_SPLINE_H

#include "command.h"
#include "editor/camera.h"
#include "editor/geometry/translation_axes.h"
#include "editor/point_selection.h"
#include "plant_generator/spline.h"

class MoveSpline : public Command {
	pg::Vec3 direction;
	pg::Vec3 totalDirection;
	TranslationAxes *axes;
	PointSelection *selection;
	pg::Spline *spline;
	pg::Vec3 planeNormal;
	pg::Ray ray;
	bool parallel;
	const Camera *camera;
	int clickOffset[2];

public:
	MoveSpline(PointSelection *selection, pg::Spline *spline,
		TranslationAxes *axes, const Camera *camera);
	void setClickOffset(int x, int y);
	void setSelection(PointSelection *selection);
	void setSpline(pg::Spline *spline);
	/** The direction that points will move in. */
	void set(pg::Ray ray, pg::Vec3 cameraDirection);
	void setParallelTangents(bool parallel);
	/** The direction and distance points will be moved in. */
	pg::Vec3 getDirection();

	void execute();
	void undo();

	bool onMouseMove(QMouseEvent *event);
	bool onMousePress(QMouseEvent *event);
	bool onMouseRelease(QMouseEvent *event);
	bool onKeyPress(QKeyEvent *event);
};

#endif /* MOVE_SPLINE_H */

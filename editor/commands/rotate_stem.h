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

#ifndef ROTATE_STEM_H
#define ROTATE_STEM_H

#include "command.h"
#include "editor/selection.h"
#include "editor/camera.h"
#include "editor/geometry/rotation_axes.h"
#include "plant_generator/plant.h"
#include <vector>

class RotateStem : public Command {
	Selection *selection;
	const Camera *camera;
	RotationAxes *axes;
	Axes::Axis axis;
	pg::Vec3 firstDirection;
	pg::Vec3 lastDirection;
	pg::Vec3 direction;
	pg::Ray ray;
	pg::Vec3 planeNormal;
	bool valid;

	void checkValidity();
	void rotateChild(pg::Stem *, pg::Quat, float);
	void rotateChildLeaves(pg::Stem *, pg::Quat, float);
	void rotateStems();
	void rotateLeaves();
	void resetRotation();
	pg::Quat getTransformation(pg::Quat);

public:
	RotateStem(Selection *selection, RotationAxes *axes,
		const Camera *camera, float x, float y);
	bool isValid();
	void set(pg::Ray ray, pg::Vec3 planeNormal);
	pg::Vec3 getDirection();
	bool onMouseMove(QMouseEvent *);
	bool onMousePress(QMouseEvent *);
	bool onKeyPress(QKeyEvent *);
	void execute();
	void undo();
};

#endif

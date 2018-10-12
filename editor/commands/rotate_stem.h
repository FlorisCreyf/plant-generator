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

#ifndef ROTATE_STEM_H
#define ROTATE_STEM_H

#include "command.h"
#include "../stem_selection.h"
#include "../geometry/rotation_axes.h"
#include "../../plant_generator/plant.h"

class RotateStem : public Command {
	StemSelection *selection;
	RotationAxes *axes;
	Axes::Axis axis;
	Axes::Axis updatedAxis;
	pg::Vec3 firstDirection;
	pg::Vec3 lastDirection;
	pg::Vec3 direction;
	pg::Ray ray;
	pg::Vec3 planeNormal;
	bool valid;

	void rotateChild(pg::Stem *stem, pg::Mat4 t, float distance);
	void rotate();
	pg::Mat4 getTransformation();
	void checkValidity();

public:
	RotateStem(StemSelection *selection, RotationAxes *axes);
	bool isValid();
	void set(pg::Ray ray, pg::Vec3 planeNormal);
	pg::Vec3 getDirection();
	void execute();
	void undo();
	RotateStem *clone();
};

#endif /* ROTATE_STEM_H */

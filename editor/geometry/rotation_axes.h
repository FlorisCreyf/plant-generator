/* Plant Genererator
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

#ifndef ROTATION_AXES_H
#define ROTATION_AXES_H

#include "axes.h"

class RotationAxes : public Axes {
public:
	Geometry getLines();
	void selectAxis(Axis axis);
	pg::Mat4 rotate(pg::Ray ray, pg::Vec3 cameraDirection,
		pg::Vec3 direction);
	pg::Mat4 getTransformation(pg::Vec3 cameraPosition, pg::Vec3 direction);
};

#endif /* ROTATION_AXES_H */

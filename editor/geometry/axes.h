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

#ifndef AXES_H
#define AXES_H

#include "geometry.h"
#include "plant_generator/math/math.h"
#include "plant_generator/math/intersection.h"

class Axes {
public:
	enum Axis {None, XAxis, YAxis, ZAxis, Center};

	/**
	 * The size for the axes needs to remain constant. This method is used
	 * to set what that size should be.
	 */
	void setScale(float scale);
	void setPosition(pg::Vec3 position);
	pg::Vec3 getPosition();
	/**
	 * The axes move along a plane parallel to the camera if the center is
	 * selected.
	 */
	void selectCenter();
	/** Selects an axis using intersection tests. */
	Axis getSelection();
	void clearSelection();
	void setScalable(bool scalable);

protected:
	float scale = 1.0f;
	pg::Vec3 position = {0.0f, 0.0f, 0.0f};
	Axis selection;
	bool scalable = true;
};

#endif /* AXES_H */

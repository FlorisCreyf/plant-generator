/* Plant Genererator
 * Copyright (C) 2018  Floris Creyf
 *
 * TreeMaker is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TreeMaker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef AXES_H
#define AXES_H

#include "plant_generator/math/math.h"
#include "plant_generator/math/intersection.h"
#include "geometry.h"

class Axes {
public:
	enum Axis {None, XAxis, YAxis, ZAxis, Center};

	Geometry getLines();
	Geometry getArrows();
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
	Axis selectAxis(pg::Ray ray);
	Axis getSelection();
	void clearSelection();
	/** Moves the axes and returns the new location. */
	pg::Vec3 move(pg::Ray ray, pg::Vec3 cameraDirection);
	/**
	 * The position of the camera is needed to keep the size of the axes
	 * the same.
	 */
	pg::Mat4 getTransformation(pg::Vec3 cameraPosition);

private:
	const float radius = 0.08f;
	const float lineLength[2] = {0.3f, 1.0f};
	const float coneLength[2] = {0.5f, 1.5f};
	float scale = 1.0f;
	pg::Vec3 position = {0.0f, 0.0f, 0.0f};
	Axis selection;
};

#endif /* AXES_H */

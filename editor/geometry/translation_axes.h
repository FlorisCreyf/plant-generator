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

#ifndef TRANSLATION_AXES_H
#define TRANSLATION_AXES_H

#include "axes.h"

class TranslationAxes : public Axes {
public:
	Geometry getLines();
	Geometry getArrows();

	/** Selects an axis using intersection tests. */
	Axis selectAxis(pg::Ray ray);
	/**
	 * The position of the camera is needed to keep the size of the axes
	 * the same.
	 */
	pg::Mat4 getTransformation(pg::Vec3 cameraPosition);

private:
	const float radius = 0.08f;
	const float lineLength[2] = {0.3f, 1.0f};
	const float coneLength[2] = {0.5f, 1.5f};
};

#endif /* TRANSLATION_AXES_H */

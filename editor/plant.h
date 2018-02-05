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

#ifndef PLANT_H
#define PLANT_H

#include "plant_generator/plant.h"
#include "plant_generator/mesh.h"
#include "plant_generator/generator.h"
#include "plant_generator/math/intersection.h"

class Plant : public pg::Plant {
	std::pair<float, pg::Stem *> getStem(pg::Ray &ray, pg::Stem *stem);

public:
	/**
	 * Creates the initial volumetric path based on where the user clicks
	 * on the screen.
	 */
	pg::Vec3 initializeStem(pg::Stem *stem, pg::Ray ray,
		pg::Vec3 cameraDirection, float position, float radius);
	pg::Vec3 extrude(pg::Stem *stem, int *point);
	pg::Stem *removePoint(pg::Stem *stem, int *point);
	pg::Stem *getStem(pg::Ray ray);
};

#endif /* PLANT_H */

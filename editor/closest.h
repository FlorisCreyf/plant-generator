/* Plant Genererator
 * Copyright (C) 2017-2018  Floris Creyf
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

#ifndef CLOSEST_H
#define CLOSEST_H

#include "camera.h"
#include "math.h"
#include <vector>

float closestDistance(std::vector<pg::Vec3> &path, Camera &cam, int x, int y);
pg::Vec3 closestPoint(std::vector<pg::Vec3> &path, Camera &cam, int x, int y);

#endif /* CLOSEST_H */

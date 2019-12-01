/* Plant Genererator
 * Copyright (C) 2016-2018  Floris Creyf
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

#ifndef FILE_H
#define FILE_H

#include "plant_generator/geometry.h"
#include <cstddef>
#include <map>
#include <vector>

class File {
public:
	void exportObj(
		const char *filename,
		const std::vector<pg::Vertex> &vertices,
		const std::vector<unsigned> &indices);
	void importObj(const char *filename, pg::Geometry *geom);
};

#endif /* FILE_H */

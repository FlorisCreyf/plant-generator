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

#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "plant_generator/math/mat4.h"
#include "plant_generator/spline.h"
#include "plant_generator/vertex.h"
#include <vector>

class Geometry {
 	std::vector<pg::DVertex> points;
	std::vector<unsigned> indices;

public:
	static const unsigned primitiveReset = 65534;
	struct Segment {
		size_t pstart;
		size_t pcount;
		size_t istart;
		size_t icount;
	};

	void clear();
	void addIndex(unsigned index);
	void addPoint(pg::Vec3 point, pg::Vec3 color);
	void addLine(pg::Vec3 line[2], pg::Vec3 color);
	void addCircle(float radius, int points, pg::Vec3 color);
	void addCurve(const pg::Spline &spline, pg::Vec3 color, int size);
	void addPlane(pg::Vec3 a, pg::Vec3 b, pg::Vec3 c,
		pg::Vec3 normal, pg::Vec3 tangent);
	void addCone(float radius, float height, int points, pg::Vec3 color);
	void addGrid(int size, pg::Vec3 pcolor[2], pg::Vec3 scolor);
	void transform(size_t start, size_t count, const pg::Mat4 &transform);
	void changeColor(size_t start, pg::Vec3 color);

	Segment getSegment() const;
	Segment append(const Geometry &geometry);

	const std::vector<pg::DVertex> *getPoints() const;
	const std::vector<unsigned> *getIndices() const;
};

#endif /* GEOMETRY_H */

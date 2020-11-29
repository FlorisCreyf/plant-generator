/* Plant Generator
 * Copyright (C) 2020  Floris Creyf
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

#ifndef SELECTOR_H
#define SELECTOR_H

#include "camera.h"
#include "selection.h"
#include "plant_generator/mesh.h"
#include <QtGui/QMouseEvent>

class Selector {
	const Camera *camera;

	bool selectPoint(const QMouseEvent *, Selection *);
	void selectMesh(const QMouseEvent *, const pg::Mesh *, Selection *);
	std::pair<float, pg::Stem *> getStem(
		pg::Ray &, pg::Stem *, pg::Plant *);
	std::pair<float, pg::Segment> getLeaf(pg::Ray, const pg::Mesh *);

public:
	Selector(const Camera *camera);
	Selector(const Selector &selector) = delete;
	Selector &operator=(const Selector &selector) = delete;
	void select(const QMouseEvent *event, const pg::Mesh *mesh,
		Selection *selection);
	int selectPoint(const QMouseEvent *event, const pg::Spline &spline,
		pg::Vec3 location, PointSelection *selection);
};

#endif

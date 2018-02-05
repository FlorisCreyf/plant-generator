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

#include "plant.h"
#include "plant_generator/patterns.h"
#include <limits>

pg::Vec3 Plant::initializeStem(pg::Stem *stem, pg::Ray ray,
	pg::Vec3 cameraDirection, float position, float radius)
{
	pg::Vec3 location;
	stem->setPosition(position);

	pg::Plane plane;
	plane.point = stem->getLocation();
	plane.normal = -1.0f * cameraDirection;
	float t = pg::intersectsPlane(ray, plane);
	location = t * ray.direction + ray.origin;

	pg::VolumetricPath path = stem->getPath();
	pg::Spline spline = path.getSpline();
	std::vector<pg::Vec3> controls;
	controls.push_back({0.0f, 0.0f, 0.0f});
	controls.push_back(location - plane.point);

	spline.setControls(controls);
	spline.setDegree(1);
	path.setSpline(spline);
	path.setMaxRadius(radius);
	path.setRadius(pg::getDefaultCurve(0));
	stem->setPath(path);

	return location;
}

pg::Vec3 Plant::extrude(pg::Stem *stem, int *point)
{
	pg::VolumetricPath path = stem->getPath();
	pg::Spline spline = path.getSpline();
	*point = spline.insert(*point, spline.getControls()[*point]);
	path.setSpline(spline);
	stem->setPath(path);
	return spline.getControls()[*point];
}

pg::Stem *Plant::removePoint(pg::Stem *stem, int *point)
{
	pg::VolumetricPath path = stem->getPath();
	pg::Spline spline = path.getSpline();
	int size = spline.getControls().size();
	if (*point <= 0 || size <= spline.getDegree() + 1) {
		if (stem->getParent()) {
			removeStem(stem);
			stem = nullptr;
			*point = -1;
		}
	} else if (size > 2) {
		spline.remove(*point);
		path.setSpline(spline);
		stem->setPath(path);
		*point = -1;
	}

	return stem;
}

pg::Stem *Plant::getStem(pg::Ray ray)
{
	pg::Stem *stem = getRoot();
	std::pair<float, pg::Stem *> selection = getStem(ray, stem);
	return selection.second;
}

std::pair<float, pg::Stem *> Plant::getStem(pg::Ray &ray, pg::Stem *stem)
{
	float max = std::numeric_limits<float>::max();
	std::pair<float, pg::Stem *> selection1(max, nullptr);
	std::pair<float, pg::Stem *> selection2(max, nullptr);

	if (stem != nullptr) {
		pg::VolumetricPath path = stem->getPath();

		for (int i = 0, j = 1; i < path.getSize()-1; i++, j++) {
			pg::Vec3 direction = path.getDirection(i);
			pg::Vec3 line[2] = {path.get(i), path.get(j)};
			line[0] = line[0] + stem->getLocation();
			line[1] = line[1] + stem->getLocation();
			float length = pg::magnitude(line[1] - line[0]);
			float r[2] = {path.getRadius(i), path.getRadius(j)};
			float t = pg::intersectsTaperedCylinder(ray, line[0],
				direction, length, r[0], r[1]);
			if (t > 0 && selection1.first > t) {
				selection1.first = t;
				selection1.second = stem;
			}
		}

		selection2 = getStem(ray, stem->getChild());
		if (selection2.second && selection2.first < selection1.first)
			selection1 = selection2;
		selection2 = getStem(ray, stem->getSibling());
		if (selection2.second && selection2.first < selection1.first)
			selection1 = selection2;
	}

	return selection1;
}

/* Plant Genererator
 * Copyright (C) 2017  Floris Creyf
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

#include "closest.h"
#include <limits>

using pg::Vec3;

 void closest(Vec3 point, const std::vector<Vec3> &path, size_t *line, float *t)
 {
	float min = std::numeric_limits<float>::max();

 	for (size_t i = 0; i < path.size() - 1; i++) {
 		Vec3 a = point - path[i];
 		Vec3 b = path[i+1] - path[i];
 		float dist = pg::project(a, b);

 		if (dist < 0.0f)
 			dist = 0.0f;
 		if (dist > 1.0f)
 			dist = 1.0f;

 		float mag = pg::magnitude(path[i] + dist * b - point);

 		if (min > mag) {
 			*t = dist;
 			min = mag;
 			*line = i;
 		}
 	}
 }

 std::vector<Vec3> toScreenSpace(const std::vector<Vec3> &path, Camera &camera)
 {
 	std::vector<Vec3> ssPath(path.size());
 	for (size_t i = 0; i < path.size(); i++) {
 		ssPath[i] = camera.toScreenSpace(path[i]);
 		ssPath[i].z = 0.0f;
 	}
 	return ssPath;
 }

 float closestDistance(std::vector<Vec3> &path, Camera &cam, int x, int y)
 {
 	float t = 0.0f;
 	size_t line = 0;
 	Vec3 point = {static_cast<float>(x), static_cast<float>(y), 0.0f};

 	closest(point, toScreenSpace(path, cam), &line, &t);

 	t *= pg::magnitude(path[line] - path[line+1]);
 	for (size_t i = 0; i < line; i++)
 		t += pg::magnitude(path[i + 1] - path[i]);

 	return t;
 }

 Vec3 closestPoint(std::vector<Vec3> &path, Camera &cam, int x, int y)
 {
 	float t = 0.0f;
 	size_t line = 0;
 	Vec3 point = {static_cast<float>(x), static_cast<float>(y), 0.0f};

 	closest(point, toScreenSpace(path, cam), &line, &t);

 	return path[line] + t*(path[line+1] - path[line]);
 }

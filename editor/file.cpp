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

#include "file.h"
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <map>

using pg::Vertex;
using pg::Vec2;
using pg::Vec3;

void File::exportObj(const char *filename,
	const std::vector<Vertex> &vertices,
	const std::vector<unsigned> &indices)
{
	FILE *f = fopen(filename, "w");

	if (f == nullptr)
		return;

	for (unsigned i = 0; i < vertices.size(); i++) {
		Vec3 position = vertices[i].position;
		fprintf(f, "v %f %f %f\n", position.x, position.y, position.z);
	}

	for (unsigned i = 0; i < vertices.size(); i++) {
		Vec2 uv = vertices[i].uv;
		fprintf(f, "vt %f %f\n", uv.x, uv.y);
	}

	for (unsigned i = 0; i < vertices.size(); i++) {
		Vec3 normal = vertices[i].normal;
		fprintf(f, "vn %f %f %f\n", normal.x, normal.y, normal.z);
	}

	for (size_t i = 0; i < indices.size(); i += 3)
		fprintf(f, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
			indices[i]+1, indices[i]+1, indices[i]+1,
			indices[i+1]+1, indices[i+1]+1, indices[i+1]+1,
			indices[i+2]+1, indices[i+2]+1, indices[i+2]+1);

	fclose(f);
}

void File::importObj(const char *filename, pg::Geometry *geom)
{
	std::ifstream file(filename);
	std::string line;

	std::vector<Vertex> points;
	std::vector<unsigned> indices;

	std::vector<pg::Vec3> vs;
	std::vector<pg::Vec3> vns;
	std::vector<pg::Vec2> vts;

	while (std::getline(file, line)) {
		std::istringstream iss(line);
		std::string field;
		iss >> field;

		if (field == "v") {
			pg::Vec3 v;
			iss >> v.x >> v.y >> v.z;
			vs.push_back(v);
		} else if (field == "vn") {
			pg::Vec3 v;
			iss >> v.x >> v.y >> v.z;
			vns.push_back(v);
		} else if (field == "vt") {
			pg::Vec2 v;
			iss >> v.x >> v.y;
			vts.push_back(v);
		}
	}

	file.clear();
	file.seekg(0, std::ios::beg);

	std::map<std::string, unsigned> fields;
	int index = 0;

	while (std::getline(file, line)) {
		std::istringstream iss(line);
		std::string field;
		iss >> field;

		if (field == "f") {
			std::vector<unsigned> shape;

			while (iss >> field) {
				auto it = fields.find(field);
				if (it == fields.end()) {
					Vertex p;
					shape.push_back(index);
					fields.emplace(field, index++);

					unsigned s[3] = {0};
					std::sscanf(
						field.c_str(), "%u/%u/%u",
						&s[0], &s[1], &s[2]);

					if (s[0] <= vs.size())
						p.position = vs[s[0] - 1];
					if (s[1] <= vns.size())
						p.normal = vns[s[1] - 1];
					if (s[2] <= vts.size())
						p.uv = vts[s[2] - 1];
					points.push_back(p);
				} else {
					shape.push_back(it->second);
				}
			}

			if (shape.size() == 3) {
				indices.push_back(shape[0]);
				indices.push_back(shape[1]);
				indices.push_back(shape[2]);
			} else if (shape.size() == 4) {
				indices.push_back(shape[0]);
				indices.push_back(shape[1]);
				indices.push_back(shape[2]);

				indices.push_back(shape[0]);
				indices.push_back(shape[2]);
				indices.push_back(shape[3]);
			}
		}
	}

	geom->setPoints(points);
	geom->setIndices(indices);
}

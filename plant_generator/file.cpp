/* Copyright 2020 Floris Creyf
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "file.h"
#include <cstdio>
#include <fstream>
#include <sstream>
#include <map>

using pg::Vertex;
using pg::Vec2;
using pg::Vec3;
using std::vector;
using std::string;

string pg::File::exportMtl(string filename, const Plant &plant)
{
	string name = filename.substr(0, filename.find_first_of("."));
	filename = name + ".mtl";
	FILE *f = fopen(filename.c_str(), "w");

	fprintf(f, "newmtl default\n");

	for (auto instance : plant.getMaterials()) {
		Material material = instance.second;
		fprintf(f, "newmtl %s\n", material.getName().c_str());
		string diffuse = material.getTexture();
		if (!diffuse.empty())
			fprintf(f, "map_Kd %s\n", diffuse.c_str());
	}

	fclose(f);
	return filename;
}

void pg::File::exportObj(
	std::string filename, const Mesh &mesh, const Plant &plant)
{
	FILE *f = fopen(filename.c_str(), "w");
	if (f == nullptr)
		return;

	fprintf(f, "mtllib %s\n", exportMtl(filename, plant).c_str());

	unsigned indexStart = 1;
	int numMeshes = mesh.getMeshCount();
	for (int m = 0; m < numMeshes; m++) {
		const vector<Vertex> *vertices = mesh.getVertices(m);
		const vector<unsigned> *indices = mesh.getIndices(m);

		long materialID = mesh.getMaterialID(m);
		if (materialID > 0) {
			Material material = plant.getMaterial(materialID);
			fprintf(f, "usemtl %s\n", material.getName().c_str());
		} else
			fprintf(f, "usemtl default\n");

		for (size_t i = 0; i < vertices->size(); i++) {
			Vec3 p = (*vertices)[i].position;
			fprintf(f, "v %f %f %f\n", p.x, p.y, p.z);
		}

		for (size_t i = 0; i < vertices->size(); i++) {
			Vec2 uv = (*vertices)[i].uv;
			fprintf(f, "vt %f %f\n", uv.x, uv.y);
		}

		for (size_t i = 0; i < vertices->size(); i++) {
			Vec3 n = (*vertices)[i].normal;
			fprintf(f, "vn %f %f %f\n", n.x, n.y, n.z);
		}

		for (size_t i = 0; i < indices->size(); i += 3)
			fprintf(f, "f %u/%u/%u %u/%u/%u %u/%u/%u\n",
				(*indices)[i]+1,
				(*indices)[i]+1,
				(*indices)[i]+1,
				(*indices)[i+1]+1,
				(*indices)[i+1]+1,
				(*indices)[i+1]+1,
				(*indices)[i+2]+1,
				(*indices)[i+2]+1,
				(*indices)[i+2]+1);

		indexStart += vertices->size();
	}
	fclose(f);
}

void insertVertexInfo(std::ifstream &file,
	vector<Vec3> &vs, vector<Vec3> &vns, vector<Vec2> &vts)
{
	std::string line;
	while (std::getline(file, line)) {
		std::istringstream iss(line);
		std::string descriptor;
		iss >> descriptor;

		if (descriptor == "v") {
			Vec3 v;
			iss >> v.x >> v.y >> v.z;
			vs.push_back(v);
		} else if (descriptor == "vn") {
			Vec3 v;
			iss >> v.x >> v.y >> v.z;
			vns.push_back(v);
		} else if (descriptor == "vt") {
			Vec2 v;
			iss >> v.x >> v.y;
			vts.push_back(v);
		}
	}
	file.clear();
	file.seekg(0, std::ios::beg);
}

void pg::File::importObj(const char *filename, pg::Geometry *geom)
{
	std::ifstream file(filename);
	std::vector<Vertex> points;
	std::vector<unsigned> indices;
	std::vector<Vec3> vs;
	std::vector<Vec3> vns;
	std::vector<Vec2> vts;
	insertVertexInfo(file, vs, vns, vts);
	/* A map is used to remove duplicate vertices. */
	std::map<std::string, unsigned> descriptors;
	std::string line;
	int index = 0;

	while (std::getline(file, line)) {
		std::istringstream iss(line);
		std::vector<unsigned> shape;
		std::string descriptor;

		iss >> descriptor;
		if (descriptor != "f")
			continue;

		while (iss >> descriptor) {
			auto it = descriptors.find(descriptor);
			if (it == descriptors.end()) {
				Vertex point;
				shape.push_back(index);
				descriptors.emplace(descriptor, index++);

				unsigned s[3] = {0};
				std::sscanf(descriptor.c_str(), "%u/%u/%u",
					&s[0], &s[1], &s[2]);

				if (s[0] <= vs.size())
					point.position = vs[s[0] - 1];
				if (s[1] <= vns.size())
					point.normal = vns[s[1] - 1];
				if (s[2] <= vts.size())
					point.uv = vts[s[2] - 1];
				points.push_back(point);
			} else
				shape.push_back(it->second);
		}

		if (shape.size() == 3) {
			/* Insert a triangle. */
			indices.push_back(shape[0]);
			indices.push_back(shape[1]);
			indices.push_back(shape[2]);
		} else if (shape.size() == 4) {
			/* Convert a rectangle into two triangles. */
			indices.push_back(shape[0]);
			indices.push_back(shape[1]);
			indices.push_back(shape[2]);

			indices.push_back(shape[0]);
			indices.push_back(shape[2]);
			indices.push_back(shape[3]);
		}
	}

	geom->setPoints(points);
	geom->setIndices(indices);
}

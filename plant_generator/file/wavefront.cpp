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

#include "wavefront.h"
#include <fstream>
#include <sstream>
#include <map>

using namespace pg;
using std::string;
using std::map;
using std::vector;
using std::ifstream;
using std::istringstream;

string Wavefront::exportMaterials(string filename, const Plant &plant)
{
	filename = filename.substr(0, filename.find_first_of(".")) + ".mtl";
	std::ofstream file;
	file.open(filename);
	if (file.fail())
		return "";

	for (Material &material : plant.getMaterials()) {
		file << "newmtl " << material.getName() << "\n";
		string diffuse = material.getTexture();
		if (!diffuse.empty())
			file << "map_Kd " << diffuse << "\n";
	}

	file.close();
	return filename;
}

void Wavefront::exportFile(
	string filename, const Mesh &mesh, const Plant &plant)
{
	std::ofstream file;
	file.open(filename);
	if (file.fail())
		return;

	file << "mtlib " << exportMaterials(filename, plant) << "\n";

	unsigned indexStart = 1;
	int numMeshes = mesh.getMeshCount();
	for (int m = 0; m < numMeshes; m++) {
		const vector<DVertex> *vertices = mesh.getVertices(m);
		const vector<unsigned> *indices = mesh.getIndices(m);

		unsigned materialIndex = mesh.getMaterialIndex(m);
		Material material = plant.getMaterial(materialIndex);
		file << "usemtl " << material.getName() << "\n";

		for (size_t i = 0; i < vertices->size(); i++) {
			Vec3 p = (*vertices)[i].position;
			file << "v " << p.x << " " << p.y << " " << p.z;
			file << "\n";
		}
		for (size_t i = 0; i < vertices->size(); i++) {
			Vec2 uv = (*vertices)[i].uv;
			file << "vt " << uv.x << " " << uv.y;
			file << "\n";
		}
		for (size_t i = 0; i < vertices->size(); i++) {
			Vec3 n = (*vertices)[i].normal;
			file << "vn " << n.x << " " << n.y << " " << n.z;
			file << "\n";
		}
		for (size_t i = 0; i < indices->size(); i += 3) {
			unsigned i1 = indices->at(i)+1;
			unsigned i2 = indices->at(i+1)+1;
			unsigned i3 = indices->at(i+2)+1;
			file << "f";
			file << " " << i1 << "/" << i1 << "/" << i1;
			file << " " << i2 << "/" << i2 << "/" << i2;
			file << " " << i3 << "/" << i3 << "/" << i3;
			file << "\n";
		}
		indexStart += vertices->size();
	}
	file.close();
}

void insertVertexInfo(ifstream &file,
	vector<Vec3> &vs, vector<Vec3> &vns, vector<Vec2> &vts)
{
	string line;
	while (std::getline(file, line)) {
		istringstream iss(line);
		string descriptor;
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

void Wavefront::importFile(const char *filename, Geometry *geom)
{
	ifstream file(filename);
	vector<DVertex> points;
	vector<unsigned> indices;
	vector<Vec3> vs;
	vector<Vec3> vns;
	vector<Vec2> vts;
	insertVertexInfo(file, vs, vns, vts);
	/* A map is used to remove duplicate vertices. */
	map<string, unsigned> descriptors;
	string line;
	int index = 0;

	while (std::getline(file, line)) {
		istringstream iss(line);
		vector<unsigned> shape;
		string descriptor;

		iss >> descriptor;
		if (descriptor != "f")
			continue;

		while (iss >> descriptor) {
			auto it = descriptors.find(descriptor);
			if (it == descriptors.end()) {
				DVertex point;
				shape.push_back(index);
				descriptors.emplace(descriptor, index++);

				unsigned s[3] = {0};
				const char *str = descriptor.c_str();
				sscanf(str, "%u/%u/%u", &s[0], &s[1], &s[2]);

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

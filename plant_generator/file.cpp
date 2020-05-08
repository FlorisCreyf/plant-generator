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
#include "thirdparty/pugixml/src/pugixml.hpp"
#include <fstream>
#include <sstream>
#include <map>
#include <iostream>
#include <algorithm>

using namespace pg;
using std::vector;
using std::map;
using std::string;
using std::ifstream;
using std::istringstream;
using pugi::xml_node;

string pg::File::exportMtl(string filename, const Plant &plant)
{
	filename = filename.substr(0, filename.find_first_of(".")) + ".mtl";
	std::ofstream file;
	file.open(filename);
	if (file.fail())
		return "";

	file << "newmtl default\n";
	for (auto instance : plant.getMaterials()) {
		Material material = instance.second;
		file << "newmtl " << material.getName() << "\n";
		string diffuse = material.getTexture();
		if (!diffuse.empty())
			file << "map_Kd " << diffuse << "\n";
	}

	file.close();
	return filename;
}

void pg::File::exportObj(string filename, const Mesh &mesh, const Plant &plant)
{
	std::ofstream file;
	file.open(filename);
	if (file.fail())
		return;

	file << "mtlib " << exportMtl(filename, plant) << "\n";

	unsigned indexStart = 1;
	int numMeshes = mesh.getMeshCount();
	for (int m = 0; m < numMeshes; m++) {
		const vector<Vertex> *vertices = mesh.getVertices(m);
		const vector<unsigned> *indices = mesh.getIndices(m);

		long materialID = mesh.getMaterialID(m);
		if (materialID > 0) {
			Material material = plant.getMaterial(materialID);
			file << "usemtl " << material.getName() << "\n";
		} else
			file << "usemtl default\n";

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

void pg::File::importObj(const char *filename, pg::Geometry *geom)
{
	ifstream file(filename);
	vector<Vertex> points;
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
				Vertex point;
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

bool isInvalidChar(char c)
{
	return !std::isalnum(c);
}

string getName(string name)
{
	if (isInvalidChar(name.at(0)))
		name.erase(name.begin());
	std::replace_if(name.begin(), name.end(), isInvalidChar, '_');
	return name;
}

string getMaterialName(long materialID, const Plant &plant)
{
	string name;
	if (materialID != 0) {
		Material material = plant.getMaterial(materialID);
		name = getName(material.getName());
	} else
		name = "default";
	return name;
}

void setMeshSources(const Mesh &mesh, xml_node meshNode)
{
	xml_node psource = meshNode.append_child("source");
	psource.append_attribute("id") = "plant-mesh-positions";
	xml_node nsource = meshNode.append_child("source");
	nsource.append_attribute("id") = "plant-mesh-normals";
	xml_node msource = meshNode.append_child("source");
	msource.append_attribute("id") = "plant-mesh-map";

	string pvalue = "";
	string nvalue = "";
	string mvalue = "";
	std::vector<Vertex> vertices = mesh.getVertices();
	for (Vertex vertex : vertices) {
		pvalue += std::to_string(vertex.position.x) + " ";
		pvalue += std::to_string(vertex.position.y) + " ";
		pvalue += std::to_string(vertex.position.z) + " ";
		nvalue += std::to_string(vertex.normal.x) + " ";
		nvalue += std::to_string(vertex.normal.y) + " ";
		nvalue += std::to_string(vertex.normal.z) + " ";
		mvalue += std::to_string(vertex.uv.x) + " ";
		mvalue += std::to_string(vertex.uv.y) + " ";

	}
	pvalue.pop_back();
	nvalue.pop_back();
	mvalue.pop_back();

	xml_node parray = psource.append_child("float_array");
	parray.append_attribute("id") = "plant-mesh-positions-array";
	parray.append_attribute("count") = vertices.size() * 3;
	parray.append_child(pugi::node_pcdata).set_value(pvalue.c_str());
	xml_node narray = nsource.append_child("float_array");
	narray.append_attribute("id") = "plant-mesh-normals-array";
	narray.append_attribute("count") = vertices.size() * 3;
	narray.append_child(pugi::node_pcdata).set_value(nvalue.c_str());
	xml_node marray = msource.append_child("float_array");
	marray.append_attribute("id") = "plant-mesh-map-array";
	marray.append_attribute("count") = vertices.size() * 2;
	marray.append_child(pugi::node_pcdata).set_value(mvalue.c_str());

	xml_node ptechnique = psource.append_child("technique_common");
	xml_node paccessor = ptechnique.append_child("accessor");
	paccessor.append_attribute("source") = "#plant-mesh-positions-array";
	paccessor.append_attribute("stride") = 3;
	paccessor.append_attribute("count") = vertices.size();
	xml_node ntechnique = nsource.append_child("technique_common");
	xml_node naccessor = ntechnique.append_child("accessor");
	naccessor.append_attribute("source") = "#plant-mesh-normals-array";
	naccessor.append_attribute("stride") = 3;
	naccessor.append_attribute("count") = vertices.size();
	xml_node mtechnique = msource.append_child("technique_common");
	xml_node maccessor = mtechnique.append_child("accessor");
	maccessor.append_attribute("source") = "#plant-mesh-map-array";
	maccessor.append_attribute("stride") = 2;
	maccessor.append_attribute("count") = vertices.size();

	xml_node param = paccessor.append_child("param");
	param.append_attribute("type") = "float";
	param.append_attribute("name") = "X";
	naccessor.append_copy(param);
	param = paccessor.append_child("param");
	param.append_attribute("type") = "float";
	param.append_attribute("name") = "Y";
	naccessor.append_copy(param);
	param = paccessor.append_child("param");
	param.append_attribute("type") = "float";
	param.append_attribute("name") = "Z";
	naccessor.append_copy(param);
	param = maccessor.append_child("param");
	param.append_attribute("type") = "float";
	param.append_attribute("name") = "S";
	param = maccessor.append_child("param");
	param.append_attribute("type") = "float";
	param.append_attribute("name") = "T";
}

void setMesh(const Mesh &mesh, const Plant &plant, xml_node meshNode)
{
	setMeshSources(mesh, meshNode);

	pugi::xml_node vertices = meshNode.append_child("vertices");
	vertices.append_attribute("id") = "plant-mesh-vertices";
	pugi::xml_node vertinput = vertices.append_child("input");
	vertinput.append_attribute("semantic") = "POSITION";
	vertinput.append_attribute("source") = "#plant-mesh-positions";

	for (int i = 0; i < mesh.getMeshCount(); i++) {
		if (mesh.getVertices(i)->size() == 0)
			continue;

		long materialID = mesh.getMaterialID(i);
		string name = getMaterialName(materialID, plant) + "-material";
		const vector<unsigned> *indices = mesh.getIndices(i);

		pugi::xml_node triangles = meshNode.append_child("triangles");
		triangles.append_attribute("material") = name.c_str();
		triangles.append_attribute("count") = indices->size() / 3;
		pugi::xml_node input = triangles.append_child("input");
		input.append_attribute("semantic") = "VERTEX";
		input.append_attribute("source") = "#plant-mesh-vertices";
		input.append_attribute("offset") = 0;
		input = triangles.append_child("input");
		input.append_attribute("semantic") = "NORMAL";
		input.append_attribute("source") = "#plant-mesh-normals";
		input.append_attribute("offset") = 1;
		input = triangles.append_child("input");
		input.append_attribute("semantic") = "TEXCOORD";
		input.append_attribute("source") = "#plant-mesh-map";
		input.append_attribute("offset") = 2;
		input.append_attribute("set") = 0;

		string value = "";
		for (unsigned index : *indices) {
			string s = std::to_string(index);
			value += s + " " + s + " " + s + " ";
		}

		xml_node p = triangles.append_child("p");
		p.append_child(pugi::node_pcdata).set_value(value.c_str());
	}
}

void setScene(xml_node scenelib, const Mesh &mesh, const Plant &plant)
{
	xml_node scene = scenelib.append_child("visual_scene");
	scene.append_attribute("id") = "scene";
	scene.append_attribute("name") = "scene";
	xml_node node = scene.append_child("node");
	node.append_attribute("id") = "plant";
	node.append_attribute("name") = "plant";
	node.append_attribute("type") = "NODE";
	xml_node geom = node.append_child("instance_geometry");
	geom.append_attribute("url") = "#plant-mesh";
	geom.append_attribute("name") = "plant";
	xml_node material = geom.append_child("bind_material");
	for (int i = 0; i < mesh.getMeshCount(); i++) {
		if (mesh.getVertices(i)->size() == 0)
			continue;
		string name = getMaterialName(mesh.getMaterialID(i), plant);
		name += "-material";
		string id = "#" + name;
		xml_node technique = material.append_child("technique_common");
		xml_node instance = technique.append_child("instance_material");
		instance.append_attribute("symbol") = name.c_str();
		instance.append_attribute("target") = id.c_str();
		xml_node input = instance.append_child("bind_vertex_input");
		input.append_attribute("semantic") = "UVMap";
		input.append_attribute("input_semantic") = "TEXCOORD";
		input.append_attribute("input_set") = 0;
	}
}

bool setTexture(long id, xml_node effectlib, xml_node imagelib, string name,
	const Plant &plant)
{
	if (id == 0)
		return false;

	Material material = plant.getMaterial(id);
	if (material.getTexture().empty())
		return false;

	string path = material.getTexture();
	string texture = getName(path);
	xml_node effect = effectlib.append_child("effect");
	effect.append_attribute("id") = name.c_str();
	xml_node profile = effect.append_child("profile_COMMON");

	string surfaceName = texture + "-surface";
	xml_node param = profile.append_child("newparam");
	param.append_attribute("sid") = surfaceName.c_str();
	xml_node surface = param.append_child("surface");
	surface.append_attribute("type") = "2D";
	xml_node init = surface.append_child("init_from");
	init.append_child(pugi::node_pcdata).set_value(texture.c_str());

	string samplerName = texture + "-sampler";
	param = profile.append_child("newparam");
	param.append_attribute("sid") = samplerName.c_str();
	xml_node sampler = param.append_child("sampler2D");
	xml_node source = sampler.append_child("source");
	xml_node sourceData = source.append_child(pugi::node_pcdata);
	sourceData.set_value(surfaceName.c_str());

	xml_node image = imagelib.append_child("image");
	image.append_attribute("id") = texture.c_str();
	image.append_attribute("name") = texture.c_str();
	init = image.append_child("init_from");
	init.append_child(pugi::node_pcdata).set_value(path.c_str());

	xml_node technique = profile.append_child("technique");
	technique.append_attribute("sid") = "common";
	xml_node lambert = technique.append_child("lambert");
	xml_node diffuse = lambert.append_child("diffuse");
	xml_node diffusetex = diffuse.append_child("texture");
	diffusetex.append_attribute("texture") = samplerName.c_str();
	diffusetex.append_attribute("texcoord") = "UVMap";

	return true;
}

void setMaterials(xml_node root, const Mesh &mesh, const Plant &plant)
{
	xml_node effectlib = root.append_child("library_effects");
	xml_node imagelib = root.append_child("library_images");
	xml_node matlib = root.append_child("library_materials");
	for (int i = 0; i < mesh.getMeshCount(); i++) {
		if (mesh.getVertices(i)->size() == 0)
			continue;
		xml_node material = matlib.append_child("material");
		long materialID = mesh.getMaterialID(i);
		string name = getMaterialName(materialID, plant);
		string id = name + "-material";
		material.append_attribute("id") = id.c_str();
		material.append_attribute("name") = id.c_str();
		id = name + "-effect";
		if (setTexture(materialID, effectlib, imagelib, id, plant)) {
			string effectID = "#" + id;
			xml_node ie = material.append_child("instance_effect");
			ie.append_attribute("url") = effectID.c_str();
		}
	}
}

void pg::File::exportDae(string filename, const Mesh &mesh, const Plant &plant)
{
	pugi::xml_document doc;
	xml_node root = doc.append_child("COLLADA");
	root.append_attribute("xmlns") =
		"http://www.collada.org/2005/11/COLLADASchema";
	root.append_attribute("xmlns:xsi") =
		"http://www.w3.org/2001/XMLSchema-instance";
	root.append_attribute("version") = "1.4.1";
	xml_node asset = root.append_child("asset");
	xml_node axis = asset.append_child("up_axis");
	axis.append_child(pugi::node_pcdata).set_value("Y_UP");

	setMaterials(root, mesh, plant);

	xml_node geolib = root.append_child("library_geometries");
	xml_node geom = geolib.append_child("geometry");
	geom.append_attribute("id") = "plant-mesh";
	geom.append_attribute("name") = "plant";
	xml_node meshNode = geom.append_child("mesh");
	setMesh(mesh, plant, meshNode);

	xml_node scenelib = root.append_child("library_visual_scenes");
	setScene(scenelib, mesh, plant);
	xml_node scene = root.append_child("scene");
	xml_node ivs = scene.append_child("instance_visual_scene");
	ivs.append_attribute("url") = "#scene";

	doc.save_file(filename.c_str());
}

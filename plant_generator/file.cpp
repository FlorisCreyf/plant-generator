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
#include "xml_writer.h"
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

string pg::File::exportMtl(string filename, const Plant &plant)
{
	filename = filename.substr(0, filename.find_first_of(".")) + ".mtl";
	std::ofstream file;
	file.open(filename);
	if (file.fail())
		return "";

	file << "newmtl default\n";
	for (Material &material : plant.getMaterials()) {
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

		unsigned materialIndex = mesh.getMaterialIndex(m);
		if (materialIndex > 0) {
			Material material = plant.getMaterial(materialIndex);
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

template<class T>
string toString(T x) {
	std::ostringstream out;
	out << x;
	return out.str();
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

string getMaterialName(unsigned materialIndex, const Plant &plant)
{
	if (materialIndex != 0) {
		Material material = plant.getMaterial(materialIndex);
		return getName(material.getName());
	} else
		return "default";
}

void setSources(XMLWriter &xml, const Mesh &mesh)
{
	string value;
	std::vector<Vertex> vertices = mesh.getVertices();

	value.clear();
	for (Vertex vertex : vertices) {
		value += toString(vertex.position.x) + " ";
		value += toString(vertex.position.y) + " ";
		value += toString(vertex.position.z) + " ";
	}
	value.pop_back();
	xml >> "<source id='plant-mesh-positions'>";
	xml += ("<float_array id='plant-mesh-positions-array' "
		"count='" + toString(vertices.size() * 3) + "'>" +
		value + "</float_array>");
	xml >> "<technique_common>";
	xml >> ("<accessor source='#plant-mesh-positions-array' stride='3' "
		"count='" + toString(vertices.size()) + "'>");
	xml += "<param type='float' name='X'/>";
	xml += "<param type='float' name='Y'/>";
	xml += "<param type='float' name='Z'/>";
	xml << "</accessor>";
	xml << "</technique_common>";
	xml << "</source>";

	value.clear();
	for (Vertex vertex : vertices) {
		value += toString(vertex.normal.x) + " ";
		value += toString(vertex.normal.y) + " ";
		value += toString(vertex.normal.z) + " ";
	}
	value.pop_back();
	xml >> "<source id='plant-mesh-normals'>";
	xml += ("<float_array id='plant-mesh-normals-array' "
		"count='" + toString(vertices.size() * 3) + "'>" +
		value + "</float_array>");
	xml >> "<technique_common>";
	xml >> ("<accessor source='#plant-mesh-normals-array' stride='3' "
		"count='" + toString(vertices.size()) + "'>");
	xml += "<param type='float' name='X'/>";
	xml += "<param type='float' name='Y'/>";
	xml += "<param type='float' name='Z'/>";
	xml << "</accessor>";
	xml << "</technique_common>";
	xml << "</source>";

	value.clear();
	for (Vertex vertex : vertices) {
		value += toString(vertex.uv.x) + " ";
		value += toString(vertex.uv.y) + " ";
	}
	value.pop_back();
	xml >> "<source id='plant-mesh-map'>";
	xml += ("<float_array id='plant-mesh-map-array' "
		"count='" + toString(vertices.size() * 2) + "'>" +
		value + "</float_array>");
	xml >> "<technique_common>";
	xml >> ("<accessor source='#plant-mesh-map-array' stride='2' "
		"count='" + toString(vertices.size()) + "'>");
	xml += "<param type='float' name='S'/>";
	xml += "<param type='float' name='T'/>";
	xml << "</accessor>";
	xml << "</technique_common>";
	xml << "</source>";
}

void setGeometry(XMLWriter &xml, const Mesh &mesh, const Plant &plant)
{
	xml >> "<library_geometries>";
	xml >> "<geometry id='plant-mesh' name='plant'>";
	xml >> "<mesh>";
	setSources(xml, mesh);
	xml >> "<vertices id='plant-mesh-vertices'>";
	xml += "<input semantic='POSITION' source='#plant-mesh-positions'/>";
	xml << "</vertices>";

	for (size_t i = 0; i < mesh.getMeshCount(); i++) {
		if (mesh.getVertices(i)->size() == 0)
			continue;

		unsigned index = mesh.getMaterialIndex(i);
		string name = getMaterialName(index, plant) + "-material";
		const vector<unsigned> *indices = mesh.getIndices(i);
		string value;
		for (unsigned index : *indices) {
			string s = toString(index);
			value += s + " " + s + " " + s + " ";
		}

		xml >> ("<triangles material='" + name + "' "
			"count='" + toString(indices->size() / 3) + "'>");
		xml += "<input semantic='VERTEX' "
			"source='#plant-mesh-vertices' offset='0'/>";
		xml += "<input semantic='NORMAL' "
			"source='#plant-mesh-normals' offset='1'/>";
		xml += "<input semantic='TEXCOORD' "
			"source='#plant-mesh-map' offset='2'/>";
		xml += "<p>" + value + "</p>";
		xml << "</triangles>";
	}

	xml << "</mesh>";
	xml << "</geometry>";
	xml << "</library_geometries>";
}

void setImages(XMLWriter &xml, const Mesh &mesh, const Plant &plant)
{
	xml >> "<library_images>";
	for (size_t i = 0; i < mesh.getMeshCount(); i++) {
		if (mesh.getVertices(i)->size() == 0)
			continue;

		unsigned materialIndex = mesh.getMaterialIndex(i);
		if (materialIndex == 0)
			continue;

		Material material = plant.getMaterial(materialIndex);
		if (material.getTexture().empty())
			continue;

		string path = material.getTexture();
		string texture = getName(path);

		xml >> ("<image id='" + texture + "' name='" + texture + "'>");
		xml += "<init_from>" + path + "</init_from>";
		xml << "</image>";
	}
	xml << "</library_images>";
}

void setEffects(XMLWriter &xml, const Mesh &mesh, const Plant &plant)
{
	xml >> "<library_effects>";
	for (size_t i = 0; i < mesh.getMeshCount(); i++) {
		if (mesh.getVertices(i)->size() == 0)
			continue;

		unsigned materialIndex = mesh.getMaterialIndex(i);
		if (materialIndex == 0)
			continue;

		Material material = plant.getMaterial(materialIndex);
		if (material.getTexture().empty())
			continue;

		std::string name = getMaterialName(materialIndex, plant);
		string path = material.getTexture();
		string texture = getName(path);

		xml >> ("<effect id='" + name + "-effect'>");
		xml >> "<profile_COMMON>";

		xml >> ("<newparam sid='" + texture + "-surface'>");
		xml >> "<surface type='2D'>";
		xml += ("<init_from>" + texture + "</init_from>");
		xml << "</surface>";
		xml << "</newparam>";

		xml >> ("<newparam sid='" + texture + "-sampler'>");
		xml >> "<sampler2D>";
		xml += ("<source>" + texture + "-surface</source>");
		xml << "</sampler2D>";
		xml << "</newparam>";

		xml >> "<technique sid='common'>";
		xml >> "<lambert>";
		xml >> "<diffuse>";
		xml += ("<texture texcoord='UVMap' "
			"texture='" + texture + "-sampler'/>");
		xml << "</diffuse>";
		xml << "</lambert>";
		xml << "</technique>";

		xml << "</profile_COMMON>";
		xml << "</effect>";
	}
	xml << "</library_effects>";
}

void setMaterials(XMLWriter &xml, const Mesh &mesh, const Plant &plant)
{
	xml >> "<library_materials>";
	for (size_t i = 0; i < mesh.getMeshCount(); i++) {
		if (mesh.getVertices(i)->size() == 0)
			continue;

		unsigned materialIndex = mesh.getMaterialIndex(i);
		string name = getMaterialName(materialIndex, plant);
		xml >> ("<material id='" + name + "-material' "
			"name='" + name + "-material'>");

		if (materialIndex != 0) {
			Material material = plant.getMaterial(materialIndex);
			if (!material.getTexture().empty()) {
				string url = "#" + name + "-effect";
				xml += "<instance_effect url='" + url + "'/>";
			}
		}

		xml << "</material>";
	}
	xml << "</library_materials>";
}

/** Create a list of bind poses that is ordered by joint ID.*/
void getJointPoses(const Stem *stem, vector<Vec3> &poses, vector<int> &ids)
{
	vector<Joint> joints = stem->getJoints();
	for (Joint joint : joints) {
		Vec3 location = joint.getLocation() + stem->getLocation();
		int id = joint.getID();
		auto it = std::upper_bound(ids.begin(), ids.end(), id);
		size_t index = it - ids.begin();
		ids.insert(it, id);
		poses.insert(poses.begin() + index, -1.0f * location);
	}

	const Stem *child = stem->getChild();
	while (child) {
		getJointPoses(child, poses, ids);
		child = child->getSibling();
	}
}

void setControllerSources(
	XMLWriter &xml, const Mesh &mesh, const Plant &plant)
{
	vector<Vec3> poses;
	vector<int> ids;
	getJointPoses(plant.getRoot(), poses, ids);
	size_t jointCount = poses.size();
	string value;

	value.clear();
	for (int id : ids)
		value += "joint" + toString(id) + " ";
	value.pop_back();
	xml >> "<source id='plant-armature-names'>";
	xml += "<Name_array id='plant-armature-names-array' "
		"count='" + toString(jointCount) + "'>" +
		value + "</Name_array>";
	xml >> "<technique_common>";
	xml >> ("<accessor source='#plant-armature-names-array' "
		"count='" + toString(jointCount) + "' stride='1'>");
	xml += "<param name='JOINT' type='name'/>";
	xml << "</accessor>";
	xml << "</technique_common>";
	xml << "</source>";

	value.clear();
	for (Vec3 location : poses) {
		value += "1 0 0 " + toString(location.x) + " ";
		value += "0 1 0 " + toString(location.y) + " ";
		value += "0 0 1 " + toString(location.z) + " ";
		value += "0 0 0 1 ";
	}
	value.pop_back();
	xml >> "<source id='plant-armature-poses'>";
	xml += "<float_array id='plant-armature-poses-array' "
		"count='" + toString(jointCount * 16) + "'>" +
		value + "</float_array>";
	xml >> "<technique_common>";
	xml >> ("<accessor source='#plant-armature-poses-array' "
		"count='" + toString(jointCount) + "' "
		"stride='16'>");
	xml += "<param name='TRANSFORM' type='float4x4'/>";
	xml << "</accessor>";
	xml << "</technique_common>";
	xml << "</source>";

	value.clear();
	vector<Vertex> vertices = mesh.getVertices();
	size_t weightCount = 0;
	for (Vertex vertex : vertices) {
		value += toString(vertex.weights.x) + " ";
		weightCount++;
		if (vertex.indices.x != vertex.indices.y) {
			value += toString(vertex.weights.y) + " ";
			weightCount++;
		}
	}
	value.pop_back();
	xml >> "<source id='plant-armature-weights'>";
	xml += "<float_array id='plant-armature-weights-array' "
		"count='" + toString(weightCount) + "'>" +
		value + "</float_array>";
	xml >> "<technique_common>";
	xml >> ("<accessor source='#plant-armature-weights-array' "
		"count='" + toString(weightCount) + "' "
		"stride='1'>");
	xml += "<param name='WEIGHT' type='float'/>";
	xml << "</accessor>";
	xml << "</technique_common>";
	xml << "</source>";
}

void setControllers(XMLWriter &xml, const Mesh &mesh, const Plant &plant)
{
	xml >> "<library_controllers>";
	xml >> "<controller id='plant-armature-skin' "
		"name='plant-armature-skin'>";
	xml >> "<skin source='#plant-mesh'>";

	setControllerSources(xml, mesh, plant);
	vector<Vertex> vertices = mesh.getVertices();

	xml >> "<joints>";
	xml += "<input semantic='JOINT' source='#plant-armature-names'/>";
	xml += "<input semantic='INV_BIND_MATRIX' "
		"source='#plant-armature-poses'/>";
	xml << "</joints>";

	string pairs;
	string influences;
	size_t weightIndex = 0;
	for (size_t i = 0; i < vertices.size(); i++) {
		Vec2 indices = vertices[i].indices;
		pairs += toString(static_cast<size_t>(indices.x)) + " ";
		pairs += toString(weightIndex++) + " ";
		if (indices.x != indices.y) {
			pairs += toString(static_cast<size_t>(indices.y)) + " ";
			pairs += toString(weightIndex++) + " ";
			influences += "2 ";
		} else
			influences += "1 ";
	}
	pairs.pop_back();
	influences.pop_back();
	xml >> ("<vertex_weights count='" + toString(vertices.size()) + "'>");
	xml += "<input semantic='JOINT' source='#plant-armature-names' "
		"offset='0'/>";
	xml += "<input semantic='WEIGHT' source='#plant-armature-weights' "
		"offset='1'/>";
	xml += "<vcount>" + influences + "</vcount>";
	xml += "<v>" + pairs + "</v>";
	xml << "</vertex_weights>";

	xml << "</skin>";
	xml << "</controller>";
	xml << "</library_controllers>";
}

void addJoints(XMLWriter &xml, const Stem *stem, Vec3 prevLocation)
{
	std::vector<Joint> joints = stem->getJoints();
	for (Joint joint : joints) {
		unsigned id = joint.getID();
		xml >> ("<node type='JOINT' "
			"id='plant-armature-joint" + toString(id) + "' "
			"name='joint" + toString(id) + "' "
			"sid='joint" + toString(id) + "'>");

		Vec3 location = joint.getLocation() - prevLocation;
		location += stem->getLocation();
		string value;
		value += "1 0 0 " + toString(location.x) + " ";
		value += "0 1 0 " + toString(location.y) + " ";
		value += "0 0 1 " + toString(location.z) + " ";
		value += "0 0 0 1";
		xml += "<matrix sid='transform'>" + value + "</matrix>";

		const Stem *child = stem->getChild();
		while (child) {
			if (!child->getJoints().empty()) {
				Joint childJoint = child->getJoints().front();
				if (childJoint.getParentID() == id) {
					Vec3 location = joint.getLocation();
					addJoints(xml, child, location);
				}
			}
			child = child->getSibling();
		}

		prevLocation = joint.getLocation() + stem->getLocation();
	}
	for (size_t i = 0; i < joints.size(); i++)
		xml << "</node>";
}

void bindPlantMaterial(XMLWriter &xml, const Mesh &mesh, const Plant &plant)
{
	xml >> "<bind_material>";
	for (size_t i = 0; i < mesh.getMeshCount(); i++) {
		if (mesh.getVertices(i)->size() == 0)
			continue;

		string name = getMaterialName(mesh.getMaterialIndex(i), plant);
		xml >> "<technique_common>";
		xml >> ("<instance_material symbol='" + name + "-material' "
			"target='#" + name + "-material'>");
		xml += "<bind_vertex_input semantic='UVMap' "
			"input_semantic='TEXCOORD' input_set='0'/>";
		xml << "</instance_material>";
		xml << "</technique_common>";
	}
	xml << "</bind_material>";
}

void addPlantGeometry(XMLWriter &xml, const Mesh &mesh, const Plant &plant)
{
	xml >> "<node id='plant' name='plant' type='NODE'>";
	xml >> "<instance_geometry url='#plant-mesh' name='plant'>";
	bindPlantMaterial(xml, mesh, plant);
	xml << "</instance_geometry>";
	xml << "</node>";
}

void addPlantController(XMLWriter &xml, const Mesh &mesh, const Plant &plant)
{
	xml >> "<node id='plant-armature' type='NODE'>";
	addJoints(xml, plant.getRoot(), Vec3(0.0f, 0.0f, 0.0f));

	xml >> "<node id='plant' name='plant' type='NODE'>";
	xml >> "<instance_controller url='#plant-armature-skin'>";
	xml += "<skeleton>#plant-armature-joint0</skeleton>";
	bindPlantMaterial(xml, mesh, plant);
	xml << "</instance_controller>";
	xml << "</node>";

	xml << "</node>";
}

void setScene(XMLWriter &xml, const Mesh &mesh, const Plant &plant,
	bool exportArmature)
{
	xml >> "<library_visual_scenes>";
	xml >> "<visual_scene id='scene' name='scene'>";
	if (exportArmature)
		addPlantController(xml, mesh, plant);
	else
		addPlantGeometry(xml, mesh, plant);
	xml << "</visual_scene>";
	xml << "</library_visual_scenes>";

	xml >> "<scene>";
	xml += "<instance_visual_scene url='#scene'/>";
	xml << "</scene>";
}

void pg::File::exportDae(string filename, const Mesh &mesh, const Plant &plant)
{
	XMLWriter xml(filename.c_str());
	xml += "<?xml version='1.0'?>";
	xml >> "<COLLADA version='1.4.1' "
		"xmlns='http://www.collada.org/2005/11/COLLADASchema' "
		"xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'>";

	xml >> "<asset>";
	xml += "<up_axis>Y_UP</up_axis>";
	xml << "</asset>";

	setImages(xml, mesh, plant);
	setEffects(xml, mesh, plant);
	setMaterials(xml, mesh, plant);
	setGeometry(xml, mesh, plant);
	if (this->exportArmature)
		setControllers(xml, mesh, plant);
	setScene(xml, mesh, plant, this->exportArmature);

	xml << "</COLLADA>";
}

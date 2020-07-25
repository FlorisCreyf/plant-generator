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

#include "collada.h"
#include "xml_writer.h"
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace pg;
using std::vector;
using std::string;

template<class T>
string toString(T x)
{
	std::ostringstream out;
	out << x;
	return out.str();
}

string toString(Mat4 mat)
{
	string result;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			result += toString(mat[j][i]) + " ";
	return result;
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
	Material material = plant.getMaterial(materialIndex);
	return getName(material.getName());
}

void setSources(XMLWriter &xml, const Mesh &mesh)
{
	string value;
	std::vector<DVertex> vertices = mesh.getVertices();

	value.clear();
	for (DVertex vertex : vertices) {
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
	for (DVertex vertex : vertices) {
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
	for (DVertex vertex : vertices) {
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

		Material material = plant.getMaterial(materialIndex);
		if (!material.getTexture().empty()) {
			string url = "#" + name + "-effect";
			xml += "<instance_effect url='" + url + "'/>";
		}

		xml << "</material>";
	}
	xml << "</library_materials>";
}

void setJointAnimation(XMLWriter &xml, const Animation &animation, size_t joint)
{
	vector<KeyFrame> frames = animation.frames[joint];
	string id = "joint" + toString(joint);
	string value;

	xml >> ("<animation id='plant-animation-" + id + "' "
		"name='plant-animation-" + id + "'>");

	value.clear();
	float timestamp = 0.0f;
	for (size_t i = 0; i < frames.size(); i++) {
		value += toString(timestamp) + " ";
		timestamp += animation.timeStep / 60.0f;
	}
	value.pop_back();
	xml >> ("<source id='plant-input-" + id + "'>");
	xml += ("<float_array id='plant-input-array-" + id + "' "
		"count='" + toString(frames.size()) + "'>" + value +
		"</float_array>");
	xml >> "<technique_common>";
	xml >> ("<accessor source='#plant-input-array-" + id + "' stride='1' "
		"count='" + toString(frames.size()) + "'>");
	xml += "<param name='TIME' type='float'/>";
	xml << "</accessor>";
	xml << "</technique_common>";
	xml << "</source>";

	value.clear();
	for (KeyFrame frame : frames) {
		Mat4 transform = toMat4(frame.rotation);
		Vec3 translation = toVec3(frame.translation);
		transform = translate(translation) * transform;
		value += toString(transform);
	}
	value.pop_back();
	xml >> ("<source id='plant-output-" + id + "'>");
	xml += ("<float_array id='plant-output-array-" + id + "' "
		"count='" + toString(frames.size()*16) + "'>" + value +
		"</float_array>");
	xml >> "<technique_common>";
	xml >> ("<accessor source='#plant-output-array-" + id + "' "
		"stride='16' count='" + toString(frames.size()) + "'>");
	xml += "<param name='TRANSFORM' type='float4x4'/>";
	xml << "</accessor>";
	xml << "</technique_common>";
	xml << "</source>";

	value.clear();
	for (size_t i = 0; i < frames.size(); i++)
		value += "LINEAR ";
	value.pop_back();
	xml >> ("<source id='plant-interpolation-" + id + "'>");
	xml += ("<Name_array id='plant-interpolation-array-" + id + "' "
		"count='" + toString(frames.size()) + "'>" + value +
		"</Name_array>");
	xml >> "<technique_common>";
	xml >> ("<accessor source='#plant-interpolation-array-" + id + "' "
		"stride='1' count='" + toString(frames.size()) + "'>");
	xml += "<param name='INTERPOLATION' type='name'/>";
	xml << "</accessor>";
	xml << "</technique_common>";
	xml << "</source>";

	xml >> ("<sampler id='plant-animation-sampler-" + id + "'>");
	xml += "<input semantic='INPUT' source='#plant-input-" + id + "'/>";
	xml += "<input semantic='OUTPUT' source='#plant-output-" + id + "'/>";
	xml += ("<input semantic='INTERPOLATION' "
		"source='#plant-interpolation-" + id + "'/>");
	xml << "</sampler>";

	xml += ("<channel source='#plant-animation-sampler-" + id + "' "
		"target='plant-armature-" + id + "/transform'/>");

	xml << "</animation>";
}

void setAnimations(XMLWriter &xml, const Animation &animation)
{
	xml >> ("<library_animations>");
	xml >> "<animation id='plant-animation' name='plant-animation'>";
	size_t size = animation.frames.size();
	for (size_t i = 0; i < size; i++)
		setJointAnimation(xml, animation, i);
	xml << "</animation>";
	xml << "</library_animations>";
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
	vector<DVertex> vertices = mesh.getVertices();
	size_t weightCount = 0;
	for (DVertex vertex : vertices) {
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
	vector<DVertex> vertices = mesh.getVertices();

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

void Collada::exportFile(string filename, const Mesh &mesh, const Scene &scene)
{
	XMLWriter xml(filename.c_str());
	xml += "<?xml version='1.0'?>";
	xml >> "<COLLADA version='1.4.1' "
		"xmlns='http://www.collada.org/2005/11/COLLADASchema' "
		"xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'>";

	xml >> "<asset>";
	xml += "<up_axis>Y_UP</up_axis>";
	xml << "</asset>";

	setImages(xml, mesh, scene.plant);
	setEffects(xml, mesh, scene.plant);
	setMaterials(xml, mesh, scene.plant);
	setGeometry(xml, mesh, scene.plant);
	if (this->exportArmature) {
		setControllers(xml, mesh, scene.plant);
		setAnimations(xml, scene.animation);
	}
	setScene(xml, mesh, scene.plant, this->exportArmature);

	xml << "</COLLADA>";
}

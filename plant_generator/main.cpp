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

#include "generator.h"
#include "pattern_generator.h"
#include "mesh.h"
#include "scene.h"
#include "file/wavefront.h"
#include <iostream>
#include <fstream>
#include <string>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, char **argv)
{
	int cycles = 3;
	int nodes = 4;
	int rays = 100;
	int divisions = 100;
	float pgr = 0.5f;
	float sgr = 0.005f;
	std::string filename = "saved/default";

	po::options_description desc("Options");
	desc.add_options()
		("help,h", "show help")
		("out,o", po::value<std::string>(),
		"set the name of the output file")
		("nodes,n", po::value<int>(),
		"set the maximum number of nodes per cycle")
		("primary-growth-rate,p", po::value<float>(),
		"set the average length of internodes")
		("secondary-growth-rate,s", po::value<float>(),
		"set the average increase in radius")
		("rays,r", po::value<int>(),
		"the maximum number of rays at any height")
		("sky-divisions,d", po::value<int>(),
		"the number of height increments of the sky")
		("cycles,c", po::value<int>(), "set the number of cycles")
	;

	try {
		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		if (vm.count("help")) {
			std::cout << desc << "\n";
			return 0;
		}
		if (vm.count("cycles"))
			cycles = vm["cycles"].as<int>();
		if (vm.count("nodes"))
			nodes = vm["nodes"].as<int>();
		if (vm.count("primary-growth-rate"))
			pgr = vm["primary-growth-rate"].as<float>();
		if (vm.count("secondary-growth-rate"))
			sgr = vm["secondary-growth-rate"].as<float>();
		if (vm.count("rays"))
			rays = vm["rays"].as<int>();
		if (vm.count("sky-divisions"))
			divisions = vm["sky-divisions"].as<int>();
		if (vm.count("out"))
			filename = vm["out"].as<std::string>();
	} catch (std::exception &exc) {
		std::cerr << exc.what() << std::endl;
		return 1;
	}

	pg::Scene scene;
	scene.plant.setDefault();

#ifndef PATTERN_GENERATOR
	pg::Generator generator(&scene.plant);
	generator.setPrimaryGrowthRate(pgr);
	generator.setSecondaryGrowthRate(sgr);
	generator.setRayDensity(rays, divisions);
	generator.grow(cycles, nodes);
#else
	pg::PatternGenerator generator(&scene.plant);
	pg::ParameterTree tree = generator.getParameterTree();
	pg::ParameterRoot *root = tree.createRoot();
	std::random_device rd;
	root->setSeed(rd());
	pg::ParameterNode *node1 = tree.addChild("");
	pg::StemData data;
	data.density = 1.0f;
	data.densityCurve.setDefault(1);
	data.start = 2.0f;
	data.scale = 0.8f;
	data.length = 50.0f;
	data.radiusThreshold = 0.02f;
	data.leaf.scale = pg::Vec3(1.0f, 1.0f, 1.0f);
	data.leaf.density = 3.0f;
	data.leaf.densityCurve.setDefault(1);
	data.leaf.distance = 3.0f;
	data.leaf.rotation = 3.141f;
	node1->setData(data);
	pg::ParameterNode *node2 = tree.addChild("1");
	data.start = 1.0f;
	data.radiusThreshold = 0.01f;
	data.angleVariation = 0.2f;
	node2->setData(data);
	pg::ParameterNode *node3 = tree.addChild("1.1");
	data.density = 0.0f;
	node3->setData(data);
	generator.setParameterTree(tree);
	generator.grow();
#endif

	pg::Mesh mesh(&scene.plant);
	mesh.generate();

	pg::Wavefront obj;
	obj.exportFile((filename + ".obj").c_str(), mesh, scene.plant);

#ifdef PG_SERIALIZE
	std::ofstream stream(filename + ".plant");
	if (stream.good()) {
		boost::archive::text_oarchive oa(stream);
		oa << scene;
	}
	stream.close();
#endif

	return 0;
}

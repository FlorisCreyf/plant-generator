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

#include "plant.h"
#include "generator.h"
#include "mesh.h"
#include "file.h"
#include <iostream>
#include <string>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, char **argv)
{
	int cycles = 1;
	int nodes = 1;
	int rays = 0;
	int divisions = 0;
	float pgr = 0.0f;
	float sgr = 0.0f;
	std::string filename = "plant.obj";

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

	pg::Generator gen;
	gen.setPrimaryGrowthRate(pgr);
	gen.setSecondaryGrowthRate(sgr);
	gen.setRayDensity(rays, divisions);
	gen.grow(cycles, nodes);

	pg::Mesh mesh = pg::Mesh(gen.getPlant());
	mesh.generate();

	pg::File file;
	if (argc == 2)
		file.exportObj(argv[1], mesh, *gen.getPlant());
	else
		file.exportObj(filename.c_str(), mesh, *gen.getPlant());

	return 0;
}

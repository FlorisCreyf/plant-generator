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
	int depth = 2;
	bool hasLeaves = true;
	std::string filename = "plant.obj";

	po::options_description desc("Options");
	desc.add_options()
		("help,h", "show help")
		("out,o", po::value<std::string>(), "set the output file")
		("depth,d", po::value<int>(), "set the depth of the plant")
		("has-leaves,l", po::value<bool>(), "enable/disable leaves")
	;

	try {
		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		if (vm.count("help")) {
			std::cout << desc << "\n";
			return 0;
		}
		if (vm.count("depth"))
			depth = vm["depth"].as<int>();
		if (vm.count("has-leaves"))
			hasLeaves = vm["has-leaves"].as<bool>();
		if (vm.count("out"))
			filename = vm["out"].as<std::string>();
	} catch (std::exception &exc) {
		std::cerr << exc.what() << std::endl;
		return 1;
	}

	pg::Plant plant;
	pg::Generator gen = pg::Generator(&plant);
	gen.setMaxDepth(depth);
	gen.disableLeaves(!hasLeaves);
	gen.grow();
	pg::Mesh mesh = pg::Mesh(&plant);
	mesh.generate();

	pg::File file;
	if (argc == 2)
		file.exportObj(argv[1], mesh, plant);
	else
		file.exportObj(filename.c_str(), mesh, plant);

	return 0;
}

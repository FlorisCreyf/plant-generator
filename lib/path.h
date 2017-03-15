/* Copyright 2017 Floris Creyf
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

#ifndef TM_PATH_H
#define TM_PATH_H

#include "math.h"
#include <cstddef>
#include <vector>

class Path {
public:
	enum Type {
		LINEAR = 0,
		BEZIER = 1
	};

	void generate();

	void setControls(std::vector<treemaker::Vec3> controls);
	std::vector<treemaker::Vec3> getControls();
	void setDivisions(int divisions);
	int getDivisions();
	void setType(Type type);
	Type getType();
	bool isGenerated();

	float getLength();
	treemaker::Vec3 getDirection(int index);
	treemaker::Vec3 getGeneratedPoint(int index);
	/** 
	 * Returns {inf, inf, inf} if the supplied distance is greater than the
	 * length of the generated path.
	 */
	treemaker::Vec3 getPoint(float distance);
	std::vector<treemaker::Vec3> getPath();

private:
	std::vector<treemaker::Vec3> controls;
	std::vector<treemaker::Vec3> path;
	Type type = LINEAR;
	int divisions = 2;
	bool generated = false;

	float getLength(std::vector<treemaker::Vec3> &path);
	void generateLinear();
};

#endif /* TM_PATH_H */

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

#include "tree_impl.h"
#include "procedural_generator.h"
#include "patterns.h"
#include "math.h"
#include <cstdlib>
#include <cmath>

using namespace treemaker;

ProcGenerator::ProcGenerator(TreeImpl *tree)
{
	this->tree = tree;
}

Vec3 ProcGenerator::getStemDirection(Stem *stem)
{
	float ratio = stem->radius / stem->getParent()->radius;
	float angleX = (float)rand() / RAND_MAX * ratio * 4.0f;
	float angleY = (float)rand() / RAND_MAX;
	Mat4 rot = rotateXY(M_PI * 0.1f + angleX, angleY * M_PI * 2.0f);
	return toVec3(rot * toVec4({0.0f, 1.0f, 0.0f}, 1.0f));
}

void ProcGenerator::getDichotomousDirections(Stem *parent, Vec3 directions[2])
{
	float angleX = (float)rand() / RAND_MAX * 0.5f;
	float angleY = (float)rand() / RAND_MAX + M_PI*0.25f;

	Path path = parent->getPath();
	Vec3 normal = path.getDirection(path.getDivisions() - 1);
	Vec3 b = cross(normal, {normal.x, 0.0f, normal.z});

	b = rotateAroundAxis(b, normal, angleY);
	directions[0] = rotateAroundAxis(normal, b, angleX);
	directions[1] = rotateAroundAxis(normal, b, -angleX);
}

void ProcGenerator::setPath(Stem *stem, Vec3 direction)
{
	Path path;
	std::vector<Vec3> controls;
	Vec3 control = {0.0f, 0.0f, 0.0f};
	Vec3 variance = {0.02f, -0.05f, 0.01f};
	int controlCount = 3;
	float length = 15.0f * stem->radius;

	for (int i = 0; i <= controlCount; i++) {
		controls.push_back(control);
		control = control + length * direction;
		direction = direction + variance;
		direction = normalize(direction);
	}
	
	if (stem->getDepth() == 0)
		path.setDivisions(10);
	else
		path.setDivisions(4);

	path.setControls(controls);
	path.setType(Path::LINEAR);
	stem->setPath(path);
}

void ProcGenerator::setRadiusCurve(Stem *stem)
{
	stem->radiusCurve = getDefaultCurve(0);
}

void ProcGenerator::addLateralStems(Stem *parent, float position)
{	
	float length = parent->getPath().getLength();
	float distance = 1.0f / parent->stemDensity;

	while (position < length) {
		Stem *stem = tree->addStem(parent);
		stem->radius = parent->radius*0.5f;
		stem->radius *= std::pow(2.0f, -position/5.0f);
		if (parent->getResolution() - 4 < 5)
			stem->setResolution(5);
		else
			stem->setResolution(parent->getResolution() - 4);
		setPath(stem, getStemDirection(stem));
		stem->setPosition(position);
		stem->baseLength = stem->getPath().getLength() / 5.0f;
		setRadiusCurve(stem);
		position += distance;
	}
}

void ProcGenerator::addDichotomousStems(Stem *parent)
{
	Vec3 directions[2];
	getDichotomousDirections(parent, directions);
	tree->addDichotomousStems(parent);
	for (int i = 0; i < 2; i++) {
		Stem *stem = parent->getDichotomousStem(i);
		stem->radius = parent->radius * 0.35f / std::pow(1.0f, 0.3f);
		parent->minRadius = stem->radius;
		stem->setResolution(parent->getResolution());
		setPath(stem, directions[i]);
		stem->baseLength = stem->getPath().getLength() / 3.0f;
		setRadiusCurve(stem);
	}
}

void ProcGenerator::updateStemDensity(Stem *stem)
{
	tree->removeLateralStems(stem);
	if (stem->stemDensity > 0.0f)
		addLateralStems(stem, stem->baseLength);
}

void ProcGenerator::updateBaseLength(Stem *stem)
{
	if (stem->stemDensity == 0.0f)
		return;
	
	float distance = 1.0f / stem->stemDensity;
	float position = stem->baseLength;
	for (size_t i = 0; i < stem->getChildCount(); i++) {
		Stem *child = stem->getChild(i);
		child->setPosition(position);
		position += distance;
	}
}

void ProcGenerator::generateTree()
{
	Stem *root = tree->getRoot();
	root->stemDensity = 2.0f;
	root->setPosition(0.0f);
	root->baseLength = 1.5f;
	setPath(root, {0.0f, 1.0f, 0.0f});
	setRadiusCurve(root);
	addLateralStems(root, root->baseLength);
	addDichotomousStems(root);
}

/* Plant Generator
 * Copyright (C) 2018  Floris Creyf
 *
 * Plant Generator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.,
 *
 * Plant Generator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "save_stem.h"

using pg::Leaf;
using pg::Stem;

SaveStem::SaveStem(Selection *selection) : selection(selection)
{

}

bool SaveStem::isSameAsCurrent()
{
	if (selection) {
		auto stemInstances = this->selection->getStemInstances();
		auto leafInstances = this->selection->getLeafInstances();
		for (auto &instance : stemInstances) {
			Stem *stem = instance.first;
			auto it = this->stems.find(stem);
			if (it == this->stems.end() || it->second != *stem)
				return false;
		}
		for (auto &instance : leafInstances) {
			Stem *stem = instance.first;
			auto it = this->stems.find(stem);
			if (it == this->stems.end() || it->second != *stem)
				return false;
		}
	}
	return true;
}

void SaveStem::execute()
{
	auto stemInstances = this->selection->getStemInstances();
	auto leafInstances = this->selection->getLeafInstances();
	for (auto &instance : stemInstances)
		this->stems.emplace(instance.first, *instance.first);
	for (auto &instance : leafInstances)
		this->stems.emplace(instance.first, *instance.first);
}

void SaveStem::undo()
{
	swap();
}

void SaveStem::redo()
{
	swap();
}

void SaveStem::swap()
{
	pg::Plant *plant = this->selection->getPlant();
	size_t materialCount = plant->getMaterials().size();
	size_t meshCount = plant->getLeafMeshes().size();
	size_t curveCount = plant->getCurves().size();

	auto stemInstances = this->selection->getStemInstances();
	auto leafInstances = this->selection->getLeafInstances();
	for (auto instance : stemInstances) {
		Stem *stem = instance.first;
		auto it = this->stems.find(stem);
		if (it != this->stems.end()) {
			Stem s = *stem;
			*stem = it->second;
			it->second = s;

			if (materialCount <= stem->getMaterial(Stem::Outer))
				stem->setMaterial(Stem::Outer, 0);
			if (materialCount <= stem->getMaterial(Stem::Inner))
				stem->setMaterial(Stem::Inner, 0);
			if (curveCount <= stem->getRadiusCurve())
				stem->setRadiusCurve(0);
		}
	}
	for (auto instance : leafInstances) {
		Stem *stem = instance.first;
		auto it = this->stems.find(stem);
		if (it != this->stems.end()) {
			Stem s = *stem;
			*stem = it->second;
			it->second = s;
		}

		for (size_t index : instance.second) {
			Leaf *leaf = stem->getLeaf(index);
			if (materialCount <= leaf->getMaterial())
				leaf->setMaterial(0);
			if (meshCount <= leaf->getMesh())
				leaf->setMesh(0);
		}
	}
}

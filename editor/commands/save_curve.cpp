/* Plant Generator
 * Copyright (C) 2021  Floris Creyf
 *
 * Plant Generator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Plant Generator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "save_curve.h"

using pg::Plant;
using pg::Scene;
using pg::Curve;

SaveCurve::SaveCurve(pg::Scene *scene) : scene(scene)
{
	this->curves.clear();
	const Plant *plant = &scene->plant;
	for (const Curve &curve : plant->getCurves())
		this->curves.push_back(curve);
}

void SaveCurve::execute()
{
	Plant *plant = &scene->plant;
	std::vector<Curve> curves = plant->getCurves();
	int size1 = curves.size();
	int size2 = this->curves.size();
	for (int i = 0; i < size1 && i < size2; i++)
		plant->updateCurve(this->curves[i], i);
	for (int i = size2; i < size1; i++)
		plant->removeCurve(i);
	for (int i = size1; i < size2; i++)
		plant->addCurve(this->curves[i]);
	this->curves = curves;
}

void SaveCurve::undo()
{
	execute();
}

void SaveCurve::redo()
{
	execute();
}

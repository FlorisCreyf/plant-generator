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

#ifndef SAVE_CURVE_H
#define SAVE_CURVE_H

#include "command.h"
#include "plant_generator/scene.h"
#include <vector>

class SaveCurve : public Command {
	std::vector<pg::Curve> curves;
	pg::Scene *scene;

public:
	SaveCurve(pg::Scene *scene);
	void execute();
	void undo();
	void redo();
};

#endif

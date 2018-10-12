/* Plant Genererator
 * Copyright (C) 2018  Floris Creyf
 *
 * Plant Genererator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Plant Genererator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MOVE_STEM_H
#define MOVE_STEM_H

#include "command.h"
#include "../camera.h"
#include "../stem_selection.h"
#include "../commands/move_spline.h"
#include <map>
#include <memory>

class MoveStem : public Command {
	StemSelection *selection;
	std::map<pg::Stem *, pg::Vec3> offsets;
	Camera camera;
	pg::Vec2 cursor;
	pg::Vec2 origCursor;
	bool snap;

	void moveAlongPath(pg::Stem *stem);

public:
	MoveStem(StemSelection *selection, Camera &camera, int x, int y);
	void snapToCursor(bool snap);
	void set(int x, int y);
	void execute();
	void undo();
	MoveStem *clone();
};

#endif /* MOVE_STEM_H */

/* Plant Genererator
 * Copyright (C) 2019  Floris Creyf
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

#ifndef ADD_LEAF_H
#define ADD_LEAF_H

#include "command.h"
#include "move_stem.h"
#include "editor/selection.h"

class AddLeaf : public Command {
	Selection *selection;
	Selection prevSelection;
	pg::Leaf leaf;
	pg::Stem *stem;
	MoveStem moveStem;

public:
	AddLeaf(Selection *selection, const Camera *camera, int x, int y);
	bool onMouseMove(QMouseEvent *event);
	bool onMousePress(QMouseEvent *event);
	void execute();
	void undo();
	void redo();
};

#endif /* ADD_LEAF_H */

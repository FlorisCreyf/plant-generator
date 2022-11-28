/* Plant Generator
 * Copyright (C) 2018  Floris Creyf
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

#ifndef ADD_STEM_H
#define ADD_STEM_H

#include "command.h"
#include "move_path.h"
#include "move_stem.h"
#include "editor/camera.h"
#include "editor/selection.h"

class AddStem : public Command {
	Selection *selection;
	Selection prevSelection;
	pg::Stem extraction;
	pg::Stem *parent;
	pg::Stem *addition;
	MoveStem moveStem;
	MovePath movePath;

	void create();
	void setRadius();

public:
	AddStem(Selection *selection, TranslationAxes *axes,
		const Camera *camera, int x, int y);
	bool onMouseMove(QMouseEvent *event);
	bool onMousePress(QMouseEvent *event);
	bool onMouseRelease(QMouseEvent *event);
	void execute();
	void undo();
	void redo();
	static bool isValid(const Selection &selection);
};

#endif

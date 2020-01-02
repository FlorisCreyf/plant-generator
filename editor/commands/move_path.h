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

#ifndef MOVE_PATH_H
#define MOVE_PATH_H

#include "command.h"
#include "move_spline.h"
#include "editor/camera.h"
#include "editor/selection.h"
#include "editor/geometry/translation_axes.h"
#include "plant_generator/plant.h"

class MovePath : public Command {
	static char modifiers;
	static char key;

	const Camera *camera;
	MoveSpline moveSpline;
	TranslationAxes *axes;
	const Selection *selection;
	float clickOffset[2];
	bool undoing;

public:
	MovePath(const Selection *selection, TranslationAxes *axes,
		const Camera *camera);
	void setClickOffset(int x, int y);
	void setParallelTangents(bool parallel);
	bool onMouseMove(QMouseEvent *event);
	bool onMousePress(QMouseEvent *event);
	bool onMouseRelease(QMouseEvent *event);
	bool onKeyPress(QKeyEvent *event);
	void execute();
	void undo();
	void redo();
};

#endif /* MOVE_PATH_H */

/* TreeMaker: 3D tree model editor
 * Copyright (C) 2017  Floris Creyf
 *
 * TreeMaker is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TreeMaker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "graphics.h"
#include "geometry.h"
#include "vector.h"
#include "collision.h"

class Axis {
public:
        enum Name {X_AXIS, Y_AXIS, Z_AXIS, CENTER, NONE};

        void create(Geometry &geom);
        void setScale(float height);
        Name pickAxis(TMvec3 center, TMray ray);
        TMmat4 getModelMatrix(TMvec3 center, TMvec3 position);
        graphics::Fragment getLineFragment();
        graphics::Fragment getArrowFragment();
        TMvec3 move(Name axis, TMray ray, TMvec3 direction, TMvec3 point);
        Name getLastSelected();
        void clearLastSelected();

private:
        const float radius = 0.08f;
        const float lineLength[2] = {0.3f, 1.0f};
        const float coneLength[2] = {0.5f, 1.5f};
        float viewportScale = 1.0f;
        graphics::Fragment lines;
        graphics::Fragment arrows;
        Name lastSelected = NONE;

        Name pickClosest(float t[3]);
};

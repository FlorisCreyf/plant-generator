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
#include "math.h"
#include "intersection.h"

class Axis {
public:
        typedef treemaker::Vec3 Vec3;
        typedef treemaker::Mat4 Mat4;

        enum Name {X_AXIS, Y_AXIS, Z_AXIS, CENTER, NONE};

        void create(Geometry &geom);
        void setScale(float height);
        Name pickAxis(Vec3 center, treemaker::Ray ray);
        treemaker::Mat4 getModelMatrix(Vec3 center, Vec3 position);
        graphics::Fragment getLineFragment();
        graphics::Fragment getArrowFragment();
        Vec3 move(Name axis, treemaker::Ray ray, Vec3 direction, Vec3 point);
        Name getSelected();
        void clearSelected();

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

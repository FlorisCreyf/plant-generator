/* TreeMaker: 3D tree model ed::itor
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

#ifndef LINES_H
#define LINES_H

#include "graphics.h"
#include "intersection.h"
#include <vector>

class Geometry {
public:
        typedef treemaker::Vec3 Vec3;
        typedef treemaker::Mat4 Mat4;
        typedef graphics::Fragment Fragment;

        Fragment addGrid(int sections, Vec3 primColor, Vec3 secColor);
        Fragment addLine(std::vector<Vec3> points, Vec3 color);
        Fragment addBezier(std::vector<Vec3> points, int sections, Vec3 color);
        Fragment addBPath(std::vector<Vec3> points, int sections, Vec3 color);
        Fragment addPlane(Vec3 a, Vec3 b, Vec3 c, Vec3 color);
        Fragment addCone(float radius, float height, unsigned divisions,
                        Vec3 color);
        graphics::VertexFormat getVertexFormat();
        void transform(int start, int count, Mat4 t);

        std::vector<float> vertices;
        std::vector<unsigned short> indices;

private:
        Fragment newFragment(int vsize, int isize, GLenum type);
};

#endif /* LINES_H */

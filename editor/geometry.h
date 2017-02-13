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
#include "collision.h"
#include <vector>

class Geometry {
public:
        graphics::Fragment addGrid(int sections, TMvec3 primColor,
                        TMvec3 secColor);
        graphics::Fragment addBox(TMaabb &b, TMvec3 c);
        graphics::Fragment addLine(std::vector<TMvec3> points, TMvec3 color);
        graphics::Fragment addBezier(std::vector<TMvec3> points, int sections,
                        TMvec3 color);
        graphics::Fragment addBPath(std::vector<TMvec3> points, int sections,
                        TMvec3 color);
        graphics::Fragment addPlane(TMvec3 a, TMvec3 b, TMvec3 c, TMvec3 color);
        graphics::Fragment addCone(float radius, float height,
                        unsigned divisions, TMvec3 color);
        graphics::VertexFormat getVertexFormat();
        void transform(int start, int count, TMmat4 t);

        std::vector<float> vertices;
        std::vector<unsigned short> indices;

private:
        graphics::Fragment newFragment(int vsize, int isize, GLenum type);
};

#endif /* LINES_H */

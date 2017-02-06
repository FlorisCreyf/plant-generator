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

#ifndef SHARED_RESOURCES_H
#define SHARED_RESOURCES_H

#define GL_GLEXT_PROTOTYPES

#include <QOpenGLFunctions>

class SharedResources {
public:
        enum Program {
                MODEL_SHADER,
                LINE_SHADER,
                FLAT_SHADER,
                WIREFRAME_SHADER,
                POINT_SHADER
        };

        void create();
        GLuint getProgramName(Program program);

private:
        GLuint programs[4] = {0};
};

#endif /* SHARED_RESOURCES_H */

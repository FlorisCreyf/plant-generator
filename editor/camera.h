/* Plant Genererator
 * Copyright (C) 2016-2018  Floris Creyf
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

#ifndef CAMERA_H
#define CAMERA_H

#include "plant_generator/math/math.h"

class Camera {
public:
	enum Action {
		Zoom, Rotate, Pan, None
	};

	Camera();
	void setAction(Action action);
	void executeAction(float x, float y);
	void setStartCoordinates(float x, float y);
	void setPan(float x, float y);
	void setCoordinates(float x, float y);
	void setPerspective(float fovy, float near, float far, float aspect);
	void setWindowSize(int width, int height);
	void zoom(float y);
	pg::Vec3 getPosition();
	pg::Vec3 getDirection();
	pg::Mat4 getVP();
	pg::Vec3 getRayDirection(int x, int y);
	pg::Vec3 toScreenSpace(pg::Vec3 point);

private:
	struct Point {
		float x;
		float y;
	};
	Point posDiff;
	Point pos;
	Point start;

	Action action;

	pg::Mat4 perspective;
	pg::Vec3 up;
	pg::Vec3 eye;
	pg::Vec3 feye;
	pg::Vec3 target;
	pg::Vec3 ftarget;
	int winWidth;
	int winHeight;
	float distance;
	float fdistance;

	pg::Vec3 getCameraPosition();
	pg::Mat4 getInverseVP();
	pg::Mat4 getInversePerspective();
	pg::Mat4 getInverseLookAt();
	pg::Mat4 getLookAtMatrix(pg::Vec3 &eye, pg::Vec3 &target, pg::Vec3 &up);
};

#endif /* CAMERA_H */

/* TreeMaker: 3D tree model editor
 * Copyright (C) 2016-2017  Floris Creyf
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

#ifndef CAMERA_H
#define CAMERA_H

#include "math.h"

class Camera {
public:
	typedef treemaker::Vec3 Vec3;
	typedef treemaker::Mat4 Mat4;

	enum Action {
		ZOOM, ROTATE, PAN, NONE
	} action;

	Camera();
	void setStartCoordinates(float x, float y);
	void setPan(float x, float y);
	void setCoordinates(float x, float y);
	void setPerspective(float fovy, float near, float far, float aspect);
	void setWindowSize(int width, int height);
	void zoom(float y);
	Vec3 getPosition();
	Vec3 getDirection();
	Mat4 getVP();
	Vec3 getRayDirection(int x, int y);
	Vec3 toScreenSpace(Vec3 point);

private:
	struct Point {
		float x;
		float y;
	};
	Point posDiff;
	Point pos;
	Point start;

	Mat4 perspective;
	Vec3 up;
	Vec3 eye;
	Vec3 feye;
	Vec3 target;
	Vec3 ftarget;
	int winWidth;
	int winHeight;
	float distance;
	float fdistance;

	Vec3 getCameraPosition();
	Mat4 getInverseVP();
	Mat4 getInversePerspective();
	Mat4 getInverseLookAt();
	Mat4 getLookAtMatrix(Vec3 &eye, Vec3 &target, Vec3 &up);
};

#endif /* CAMERA_H */

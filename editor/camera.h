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
#include "plant_generator/math/intersection.h"

class Camera {
public:
	enum Action {
		Zoom, Rotate, Pan, None
	};

	Camera();
	void setPanSpeed(float speed);
	void setZoom(float speed, float min, float max);
	void setOrientation(float x, float y);
	void setDistance(float distance);
	void setTarget(pg::Vec3 target);
	void setAction(Action action);
	/** Returns true if an action was executed. */
	bool executeAction(float x, float y);
	void setStartCoordinates(float x, float y);
	void setPan(float x, float y);
	void setCoordinates(float x, float y);
	void setOrthographic(pg::Vec3 near, pg::Vec3 far);
	void setPerspective(float fovy, float near, float far, float aspect);
	void setWindowSize(int width, int height);
	void zoom(float y);
	pg::Vec3 getPosition() const;
	pg::Vec3 getDirection() const;
	pg::Vec3 getTarget() const;
	pg::Mat4 getVP() const;
	pg::Mat4 updateVP();
	pg::Vec3 toScreenSpace(pg::Vec3 point) const;
	pg::Ray getRay(int x, int y) const;
	pg::Vec3 getFar() const;
	pg::Vec3 getNear() const;
	bool isPerspective() const;

private:
	pg::Vec2 posDiff;
	pg::Vec2 pos;
	pg::Vec2 start;

	Action action;

	pg::Mat4 projection;
	pg::Vec3 eye;
	pg::Vec3 feye;
	pg::Vec3 target;
	pg::Vec3 ftarget;
	pg::Vec3 farPlane;
	pg::Vec3 nearPlane;
	int winWidth;
	int winHeight;
	float distance;
	bool perspective;

	float panSpeed;
	float zoomSpeed;
	float zoomMin;
	float zoomMax;
	float scroll;
	float prevY;

	pg::Vec3 getCameraPosition() const;
	pg::Mat4 getInverseVP() const;
	pg::Mat4 getInverseOrthographic() const;
	pg::Mat4 getInversePerspective() const;
	pg::Mat4 getInverseLookAt() const;
	pg::Mat4 getLookAtMatrix(pg::Vec3 eye, pg::Vec3 target) const;
	pg::Ray getOrthographicRay(int x, int y) const;
	pg::Ray getPerspectiveRay(int x, int y) const;
	void initOrthographic(pg::Vec3 near, pg::Vec3 far);
};

#endif /* CAMERA_H */

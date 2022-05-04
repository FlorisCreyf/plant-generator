/* Plant Generator
 * Copyright (C) 2016-2018  Floris Creyf
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

#ifndef CAMERA_H
#define CAMERA_H

#include "plant_generator/math/vec2.h"
#include "plant_generator/math/vec3.h"
#include "plant_generator/math/mat4.h"
#include "plant_generator/math/intersection.h"

#undef near
#undef far

class Camera {
public:
	enum Action {Zoom, Rotate, Pan, None};

	Camera();
	void setPanSpeed(float speed);
	void setZoom(float speed, float min, float max);
	void setOrientation(float x, float y);
	void setDistance(float distance);
	void setTarget(pg::Vec3 target);
	void setAction(Action action);
	void setWindowSize(int width, int height);
	void setOrthographic(pg::Vec3 near, pg::Vec3 far);
	void setPerspective(float fovy, float near, float far, float aspect);
	void scaleOrthographicVolume(bool scale);

	void setStartCoordinates(float x, float y);
	/** Returns true if an action was executed. */
	bool executeAction(float x, float y);
	void zoom(float y);
	void pan(float x, float y);
	void rotate(float x, float y);

	pg::Vec3 getPosition() const;
	pg::Vec3 getDirection() const;
	pg::Mat4 getTransform() const;
	pg::Ray getRay(int x, int y) const;
	pg::Vec3 toScreenSpace(pg::Vec3 point) const;
	pg::Vec3 getFar() const;
	pg::Vec3 getNear() const;
	float getDistance() const;
	bool isPerspective() const;

	pg::Mat4 getOrthographic() const;
	pg::Mat4 getInverseOrthographic() const;
	pg::Mat4 getPerspective() const;
	pg::Mat4 getInversePerspective() const;
	pg::Mat4 getView() const;
	pg::Mat4 getInverseView() const;

private:
	float xAngle;
	float zAngle;
	float distance;
	float panSpeed;
	float zoomSpeed;
	float rotateSpeed;
	float zoomMin;
	float zoomMax;
	pg::Vec2 cursor;
	pg::Vec3 target;
	pg::Vec3 far;
	pg::Vec3 near;
	Action action;
	int winWidth;
	int winHeight;
	bool perspective;
	bool scaleOrthographic;

	pg::Ray getOrthographicRay(int x, int y) const;
	pg::Ray getPerspectiveRay(int x, int y) const;
};

#endif

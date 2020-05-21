/* Copyright 2017 Floris Creyf
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "geometry.h"

using pg::Vec3;
using pg::Quat;
using pg::Vertex;

long pg::Geometry::counter = 1;

pg::Geometry::Geometry()
{
	id = counter++;
}

void pg::Geometry::clear()
{
	points.clear();
	indices.clear();
}

long pg::Geometry::getID() const
{
	return id;
}

std::string pg::Geometry::getName() const
{
	return name;
}

void pg::Geometry::setName(std::string name)
{
	this->name = name;
}

void pg::Geometry::setPlane()
{
	Vertex p;
	p.normal = Vec3(0.0f, 1.0f, 0.0f);
	points.clear();
	indices.clear();

	p.position = Vec3(0.5f, 0.0f, 0.0f);
	p.uv = Vec2(0.0f, 1.0f);
	points.push_back(p);
	p.position = Vec3(0.5f, 0.0f, 1.0f);
	p.uv = Vec2(0.0f, 0.0f);
	points.push_back(p);
	p.position = Vec3(-0.5f, 0.0f, 1.0f);
	p.uv = Vec2(1.0f, 0.0f);
	points.push_back(p);
	p.position = Vec3(-0.5f, 0.0f, 0.0f);
	p.uv = Vec2(1.0f, 1.0f);
	points.push_back(p);

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(3);

	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(3);
}

void pg::Geometry::setPerpendicularPlanes()
{
	Vertex p;
	p.normal = {0.0f, 1.0f, 0.0f};

	setPlane();

	p.position = Vec3(0.0f, 0.5f, 0.0f);
	p.uv = Vec2(0.0f, 1.0f);
	points.push_back(p);
	p.position = Vec3(0.0f, 0.5f, 1.0f);
	p.uv = Vec2(0.0f, 0.0f);
	points.push_back(p);
	p.position = Vec3(0.0f, -0.5f, 1.0);
	p.uv = Vec2(1.0f, 0.0f);
	points.push_back(p);
	p.position = Vec3(0.0f, -0.5f, 0.0);
	p.uv = Vec2(1.0f, 1.0f);
	points.push_back(p);

	indices.push_back(4);
	indices.push_back(5);
	indices.push_back(7);

	indices.push_back(5);
	indices.push_back(6);
	indices.push_back(7);
}

void pg::Geometry::setPoints(std::vector<Vertex> points)
{
	this->points = points;
}

void pg::Geometry::setIndices(std::vector<unsigned> indices)
{
	this->indices = indices;
}

const std::vector<Vertex> &pg::Geometry::getPoints() const
{
	return points;
}

const std::vector<unsigned> &pg::Geometry::getIndices() const
{
	return indices;
}

void pg::Geometry::transform(Quat rotation, Vec3 scale, Vec3 translation)
{
	for (auto &point : points) {
		point.position.x *= scale.x;
		point.position.y *= scale.y;
		point.position.z *= scale.z;

		Quat p = toQuat(point.position, 1.0f);
		Quat n = toQuat(point.normal, 1.0f);
		point.position = toVec3(rotation * p * conjugate(rotation));
		point.normal = toVec3(rotation * n * conjugate(rotation));

		point.position += translation;
	}
}

void pg::Geometry::toCenter()
{
	Vec3 avg(0.0f, 0.0f, 0.0f);
	for (auto &point : points)
		avg += point.position;
	avg /= points.size();
	for (auto &point : points)
		point.position -= avg;
}

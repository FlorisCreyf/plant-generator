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
using pg::DVertex;

void pg::Geometry::clear()
{
	points.clear();
	indices.clear();
}

std::string pg::Geometry::getName() const
{
	return this->name;
}

void pg::Geometry::setName(std::string name)
{
	this->name = name;
}

void pg::Geometry::setPlane()
{
	DVertex p;
	p.tangentScale = 1.0f;
	p.normal = Vec3(0.0f, 0.0f, 1.0f);
	p.tangent = Vec3(0.0f, 1.0f, 0.0f);
	this->points.clear();
	this->indices.clear();
	p.position = Vec3(0.5f, 0.0f, 0.0f);
	p.uv = Vec2(1.0f, 0.0f);
	this->points.push_back(p);
	p.position = Vec3(0.5f, 1.0f, 0.0f);
	p.uv = Vec2(1.0f, 1.0f);
	this->points.push_back(p);
	p.position = Vec3(-0.5f, 1.0f, 0.0f);
	p.uv = Vec2(0.0f, 1.0f);
	this->points.push_back(p);
	p.position = Vec3(-0.5f, 0.0f, 0.0f);
	p.uv = Vec2(0.0f, 0.0f);
	this->points.push_back(p);
	this->indices.push_back(0);
	this->indices.push_back(1);
	this->indices.push_back(3);
	this->indices.push_back(1);
	this->indices.push_back(2);
	this->indices.push_back(3);
}

void pg::Geometry::setPerpendicularPlanes()
{
	DVertex p;
	p.tangentScale = 1.0f;
	p.normal = Vec3(1.0f, 0.0f, 0.0f);
	p.tangent = Vec3(0.0f, 1.0f, 0.0f);
	setPlane();
	p.position = Vec3(0.0f, 0.0f, 0.5f);
	p.uv = Vec2(1.0f, 0.0f);
	this->points.push_back(p);
	p.position = Vec3(0.0f, 0.0f, -0.5f);
	p.uv = Vec2(0.0f, 0.0f);
	this->points.push_back(p);
	p.position = Vec3(0.0f, 1.0f, 0.5f);
	p.uv = Vec2(1.0f, 1.0f);
	this->points.push_back(p);
	p.position = Vec3(0.0f, 1.0f, -0.5f);
	p.uv = Vec2(0.0f, 1.0f);
	this->points.push_back(p);
	this->indices.push_back(4);
	this->indices.push_back(5);
	this->indices.push_back(7);
	this->indices.push_back(7);
	this->indices.push_back(6);
	this->indices.push_back(4);
}

void pg::Geometry::setPoints(std::vector<DVertex> points)
{
	this->points = points;
}

void pg::Geometry::setIndices(std::vector<unsigned> indices)
{
	this->indices = indices;
}

const std::vector<DVertex> &pg::Geometry::getPoints() const
{
	return this->points;
}

const std::vector<unsigned> &pg::Geometry::getIndices() const
{
	return this->indices;
}

void pg::Geometry::computeTangents()
{
	for (size_t i = 0; i < this->points.size(); i++) {
		this->points[i].tangent = Vec3(0.0f, 0.0f, 0.0f);
		this->points[i].tangentScale = 0.0f;
	}

	for (size_t i = 0; i < this->indices.size(); i += 3) {
		size_t i1 = this->indices[i+0];
		size_t i2 = this->indices[i+1];
		size_t i3 = this->indices[i+2];
		DVertex p1 = this->points[i1];
		DVertex p2 = this->points[i2];
		DVertex p3 = this->points[i3];

		Vec3 edge1 = p2.position - p1.position;
		Vec3 edge2 = p3.position - p1.position;
		Vec2 duv1 = p2.uv - p1.uv;
		Vec2 duv2 = p3.uv - p1.uv;
		float c = 1.0f / (duv1.x*duv2.y - duv1.y*duv2.x);
		Vec3 tangent;
		Vec3 bitangent;
		tangent.x = c * (duv2.y*edge1.x - duv1.y*edge2.x);
		tangent.y = c * (duv2.y*edge1.y - duv1.y*edge2.y);
		tangent.y = c * (duv2.y*edge1.z - duv1.y*edge2.z);
		bitangent.x = c * (-duv2.x*edge1.x + duv1.x*edge2.x);
		bitangent.y = c * (-duv2.x*edge1.y + duv1.x*edge2.y);
		bitangent.z = c * (-duv2.x*edge1.z + duv1.x*edge2.z);

		float m1 = magnitude(bitangent);
		float m2 = magnitude(cross(p1.normal, tangent));
		float sign = 1.0f;
		if ((m1 > 0.0f && m2 < 0.0f) || (m1 < 0.0f && m2 > 0.0f))
			sign = -1.0f;

		this->points[i1].tangent += tangent;
		this->points[i2].tangent += tangent;
		this->points[i3].tangent += tangent;
		this->points[i1].tangentScale += 1.0f;
		this->points[i2].tangentScale += 1.0f;
		this->points[i3].tangentScale += 1.0f;
		this->points[i1].tangentScale *= sign;
		this->points[i2].tangentScale *= sign;
		this->points[i3].tangentScale *= sign;
	}

	for (size_t i = 0; i < this->points.size(); i++) {
		float scale = this->points[i].tangentScale;
		float sign = copysignf(1.0f, scale);
		this->points[i].tangent /= std::abs(scale);
		this->points[i].tangent = normalize(this->points[i].tangent);
		this->points[i].tangentScale = sign;
	}
}

void pg::Geometry::transform(Quat rotation, Vec3 scale, Vec3 translation)
{
	for (auto &point : this->points) {
		point.position.x *= scale.x;
		point.position.y *= scale.y;
		point.position.z *= scale.z;
		point.position = rotate(rotation, point.position);
		point.normal = rotate(rotation, point.normal);
		point.position += translation;
	}
}

void pg::Geometry::toCenter()
{
	Vec3 avg(0.0f, 0.0f, 0.0f);
	for (auto &point : this->points)
		avg += point.position;
	avg /= this->points.size();
	for (auto &point : this->points)
		point.position -= avg;
}

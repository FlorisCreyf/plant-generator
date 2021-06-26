/* Copyright 2020 Floris Creyf
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

#include "cross_section.h"
#include <cmath>

const float pi = 3.14159265359f;

using namespace pg;

CrossSection::CrossSection() : resolution(0)
{

}

int CrossSection::getResolution() const
{
	return this->resolution;
}

void CrossSection::setSpline(Spline spline)
{
	this->spline = spline;
}

void CrossSection::generate(int resolution)
{
	this->resolution = resolution;
	if (this->spline.getSize())
		generateSpline();
	else
		generateCircle();
}

void CrossSection::generateCircle()
{
	SVertex vertex;
	vertex.uv.x = 1.0f;
	float angle = 0.0f;
	float deltaUV = 1.0f / this->resolution;
	float deltaAngle = 2.0f * pi / this->resolution;
	this->vertices.resize(this->resolution+1);
	for (int i = 0; i <= this->resolution; i++) {
		vertex.position = Vec3(std::cos(angle), std::sin(angle), 0.0f);
		vertex.normal = normalize(vertex.position);
		this->vertices[i] = vertex;
		vertex.uv.x -= deltaUV;
		angle += deltaAngle;
	}
}

void CrossSection::generateSpline()
{
	SVertex vertex;
	vertex.uv.x = 1.0f;
	int curves = this->spline.getCurveCount();
	float t = 0.0f;
	float deltaT = static_cast<float>(curves) / this->resolution;
	float deltaUV = 1.0f / this->resolution;
	this->vertices.resize(this->resolution+1);
	for (int i = 0; i <= this->resolution; i++) {
		vertex.position = this->spline.getPoint(t);
		this->vertices[i] = vertex;
		vertex.uv.x -= deltaUV;
		t += deltaT;
	}
	Vec3 position1 = this->vertices[1].position;
	Vec3 position2 = this->vertices[0].position;
	Vec3 position3 = this->vertices[this->resolution].position;
	Vec3 normal1 = normalize(position2 - position1);
	Vec3 normal2 = normalize(position3 - position2);
	this->vertices[0].normal = normalize(normal1 + normal2);
	this->vertices[this->resolution].normal = normalize(normal1 + normal2);
	for (int i = 1; i < this->resolution; i++) {
		position1 = this->vertices[i-1].position;
		position2 = this->vertices[i].position;
		position3 = this->vertices[i+1].position;
		normal1 = normalize(position2 - position1);
		normal2 = normalize(position3 - position2);
		this->vertices[i].normal = normalize(normal1 + normal2);
	}
}

void CrossSection::scale(float x, float y)
{
	for (size_t i = 0, size = this->vertices.size(); i < size; i++) {
		this->vertices[i].position.x *= x;
		this->vertices[i].position.z *= y;
	}
}

void CrossSection::setVertices(std::vector<SVertex> vertices)
{
	this->vertices = vertices;
}

std::vector<SVertex> CrossSection::getVertices() const
{
	return this->vertices;
}

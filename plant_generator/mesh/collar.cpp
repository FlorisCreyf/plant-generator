/* Copyright 2022 Floris Creyf
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

#include "collar.h"
#include "util.h"

using namespace pg;

const float pi = 3.14159265359f;

Collar::Collar(Plant *plant, Mesh &mesh) : plant(plant), mesh(mesh)
{

}

/** Add a subsequent triangle ring to connect the collar with the stem. */
void Collar::connectCollar(const Mesh::State &state, bool fork)
{
	Stem *stem = state.segment.stem;
	size_t sections = state.segment.stem->getPath().getSize();
	bool a = !fork && state.section > 0 && state.section < sections;
	bool b = fork && state.section+1 < sections;
	if (a || b)
		this->mesh.addTriangleRing(
			state.prevIndex,
			this->mesh.vertices[state.mesh].size(),
			stem->getSectionDivisions(), state.mesh);
}

/** Return the amount of memory needed for the branch collar. */
size_t Collar::getBranchCollarSize(Stem *stem)
{
	int cd = stem->getPath().getInitialDivisions();
	return (stem->getSectionDivisions()+1) * cd;
}

/** Cross sections are usually created one at a time and then connected with
triangles. Branch collars are created by connecting cross sections with
splines. Reserving memory in advance enables an identical vertex layout. */
void Collar::reserveBranchCollarSpace(Stem *stem, int mesh)
{
	size_t csize = getBranchCollarSize(stem);
	size_t vsize = this->mesh.vertices[mesh].size();
	this->mesh.vertices[mesh].resize(vsize + csize);
}

/** The first step in generating the branch collar is scaling the first cross
section of the stem along the parent stem. */
Mat4 Collar::getBranchCollarScale(Stem *child, Stem *parent)
{
	Mat4 scale = identity();
	if (!parent) {
		scale[0][0] = child->getSwelling().x;
		scale[1][1] = child->getSwelling().y;
		return scale;
	}
	scale[0][0] = child->getSwelling().x;
	scale[2][2] = child->getSwelling().y;
	float position = child->getDistance();
	Vec3 zaxis = parent->getPath().getIntermediateDirection(position);
	Vec3 yaxis = child->getPath().getDirection(0);
	Vec3 xaxis = normalize(cross(zaxis, yaxis));
	yaxis = cross(xaxis, zaxis);
	Mat4 basis = identity();
	basis.vectors[0] = toVec4(xaxis, 0.0f);
	basis.vectors[1] = toVec4(yaxis, 0.0f);
	basis.vectors[2] = toVec4(zaxis, 0.0f);
	return basis * scale * transpose(basis);
}

Vec3 Collar::getSurfaceNormal(Vec3 p1, Vec3 p2, Vec3 p3,
	Vec3 n1, Vec3 n2, Vec3 n3, Vec3 intersection)
{
	Vec3 edge1 = p2 - p1;
	Vec3 edge2 = p3 - p1;
	float m1 = 1.0f / magnitude(edge1);
	float m2 = 1.0f / magnitude(edge2);
	intersection -= p1;
	n1 = lerp(n1, n2, project(intersection, m1*edge1) * m1);
	n2 = lerp(n1, n3, project(intersection, m2*edge2) * m2);
	return normalize(normalize(n1) + normalize(n2));
}

/** Return a starting point for branch collar triangle intersections. */
size_t Collar::getTriangleOffset(
	const Mesh::Segment &parent, const Mesh::Segment &child)
{
	size_t offset = 0;
	if (parent.stem) {
		const Path &path = parent.stem->getPath();
		size_t pathIndex = path.getIndex(child.stem->getDistance());
		size_t sectionDivisions = parent.stem->getSectionDivisions();
		offset = pathIndex * sectionDivisions * 6;
		offset += sectionDivisions * 3;
		if (offset > parent.indexCount)
			offset = parent.indexCount - 3;
		offset += parent.indexStart;
	}
	return offset;
}

void Collar::insertCurve(Vec3 c[4], int degree, DVertex v,
	int cdivisions, int sdivisions, DVertex *buffer)
{
	float delta = 1.0f / (cdivisions + 1);
	float t = delta;
	if (degree == 3)
		for (int i = 0; i < cdivisions; i++, t += delta) {
			v.position = getCubicBezier(t, c[0], c[1], c[2], c[3]);
			buffer[sdivisions*i] = v;
		}
	else
		for (int i = 0; i < cdivisions; i++, t += delta) {
			v.position = getQuadraticBezier(t, c[0], c[1], c[3]);
			buffer[sdivisions*i] = v;
		}
}

size_t Collar::insertCollar(
	Mesh::Segment child, Mesh::Segment parent, size_t vertexStart)
{
	const int mesh = child.stem->getMaterial(Stem::Outer);
	const Path &path = child.stem->getPath();
	const int sdivisions = child.stem->getSectionDivisions() + 1;
	const int cdivisions = path.getInitialDivisions();
	size_t collarSize = getBranchCollarSize(child.stem);
	Mat4 scale = getBranchCollarScale(child.stem, parent.stem);
	size_t offset = getTriangleOffset(parent, child);

	Vec3 direction(0.0f);
	int degree = path.getSpline().getDegree();
	if (degree == 3) {
		auto controls = path.getSpline().getControls();
		direction = controls[3] - controls[2];
	}

	for (int i = 0; i < sdivisions; i++) {
		size_t index = child.vertexStart + i;
		size_t index2 = index + collarSize + sdivisions;
		DVertex v1 = this->mesh.vertices[mesh][index];
		DVertex v2;
		Ray ray;
		v2.normal = v1.normal;
		v2.tangent = v1.tangent;
		v2.tangentScale = v1.tangentScale;
		v2.position = v1.position - child.stem->getLocation();
		v2.position = scale.apply(v2.position, 1.0f);
		v2.position += child.stem->getLocation();
		ray.origin = this->mesh.vertices[mesh][index2].position;
		ray.direction = normalize(v2.position - ray.origin);
		v2 = moveToSurface(v2, ray, parent, offset);
		if (std::isinf(v2.position.x)) {
			this->mesh.vertices[mesh].resize(child.vertexStart);
			this->mesh.indices[mesh].resize(child.indexStart);
			return 0;
		}
		v2.weights = this->mesh.vertices[mesh][index].weights;
		v2.indices = this->mesh.vertices[mesh][index].indices;
		this->mesh.vertices[mesh][index] = v2;
		ray.direction = normalize(v1.position - ray.origin);
		v1 = moveToSurface(v1, ray, parent, offset);
		if (std::isinf(v1.position.x)) {
			this->mesh.vertices[mesh].resize(child.vertexStart);
			this->mesh.indices[mesh].resize(child.indexStart);
			return 0;
		}
		Vec3 curve[4];
		curve[0] = v2.position;
		curve[1] = v1.position;
		curve[2] = ray.origin - direction;
		curve[3] = ray.origin;
		DVertex *buffer = &this->mesh.vertices[mesh][vertexStart+i];
		insertCurve(curve, degree, v2, cdivisions, sdivisions, buffer);
	}

	size_t index1 = child.vertexStart;
	size_t index2 = child.vertexStart + sdivisions;
	for (int i = 0; i <= cdivisions; i++) {
		this->mesh.addTriangleRing(index1, index2, sdivisions-1, mesh);
		index1 = index2;
		index2 += sdivisions;
	}

	index1 = child.vertexStart;
	index2 = vertexStart + collarSize;
	setBranchCollarNormals(index1, index2, mesh, sdivisions, cdivisions);
	setBranchCollarUVs(index2, child.stem, mesh, sdivisions, cdivisions);
	return cdivisions + 2;
}

/** Project a point from a cross section on its parent's surface. */
DVertex Collar::moveToSurface(
	DVertex vertex, Ray ray, Mesh::Segment parent, size_t firstIndex)
{
	if (!parent.stem) {
		Plane plane;
		plane.point = Vec3(0.0f);
		plane.normal = Vec3(0.0f, 0.0f, 1.0f);
		float t = intersectsPlane(ray, plane);
		vertex.position = ray.origin + t * ray.direction;
		return vertex;
	}

	unsigned mesh = parent.stem->getMaterial(Stem::Outer);
	DVertex *vertices = &this->mesh.vertices[mesh][0];
	unsigned *indices = &this->mesh.indices[mesh][0];
	size_t lastIndex = parent.indexStart + parent.indexCount;
	float t = 0.0f;

	for (size_t offset = 0; offset < parent.indexCount; offset += 3) {
		size_t i = firstIndex + offset;
		if (i < lastIndex) {
			Vec3 p1 = vertices[indices[i+0]].position;
			Vec3 p2 = vertices[indices[i+1]].position;
			Vec3 p3 = vertices[indices[i+2]].position;
			t = intersectsFrontTriangle(ray, p1, p2, p3);
			if (t != 0.0f) {
				vertex.normal = getSurfaceNormal(p1, p2, p3,
					vertices[indices[i+0]].normal,
					vertices[indices[i+1]].normal,
					vertices[indices[i+2]].normal,
					t*ray.direction + ray.origin);
				break;
			}
		}
		i = firstIndex - offset;
		if (i >= parent.indexStart && offset <= firstIndex) {
			Vec3 p1 = vertices[indices[i+0]].position;
			Vec3 p2 = vertices[indices[i+1]].position;
			Vec3 p3 = vertices[indices[i+2]].position;
			t = intersectsFrontTriangle(ray, p1, p2, p3);
			if (t != 0.0f) {
				vertex.normal = getSurfaceNormal(p1, p2, p3,
					vertices[indices[i+0]].normal,
					vertices[indices[i+1]].normal,
					vertices[indices[i+2]].normal,
					t*ray.direction + ray.origin);
				break;
			}
		}
	}

	if (t == 0.0f)
		return moveToForkSurface(vertex, ray, parent);
	else {
		vertex.normal = normalize(vertex.normal);
		vertex.position = ray.origin + t * ray.direction;
		return vertex;
	}
}

DVertex Collar::moveToForkSurface(DVertex vertex, Ray ray, Mesh::Segment parent)
{
	Stem *fork[2] = {};
	parent.stem->getFork(fork);
	if (!fork[0]) {
		vertex.position = Vec3(std::numeric_limits<float>::infinity());
		return vertex;
	}

	float t = 0.0f;
	Mesh::Segment segment1 = this->mesh.findStem(fork[0]);
	Mesh::Segment segment2 = this->mesh.findStem(fork[1]);
	unsigned mesh1 = fork[0]->getMaterial(Stem::Outer);
	unsigned mesh2 = fork[1]->getMaterial(Stem::Outer);
	DVertex *vertices1 = &this->mesh.vertices[mesh1][0];
	DVertex *vertices2 = &this->mesh.vertices[mesh2][0];
	unsigned *indices1 = &this->mesh.indices[mesh1][0];
	unsigned *indices2 = &this->mesh.indices[mesh2][0];
	size_t collarSize = fork[0]->getPath().getInitialDivisions();
	collarSize *= fork[0]->getSectionDivisions() * 6;

	for (size_t offset = 0; offset < collarSize; offset += 3) {
		size_t i = segment1.indexStart + offset;
		Vec3 p1 = vertices1[indices1[i+0]].position;
		Vec3 p2 = vertices1[indices1[i+1]].position;
		Vec3 p3 = vertices1[indices1[i+2]].position;
		t = intersectsFrontTriangle(ray, p1, p2, p3);
		if (t != 0.0f) {
			vertex.normal = getSurfaceNormal(p1, p2, p3,
				vertices1[indices1[i+0]].normal,
				vertices1[indices1[i+1]].normal,
				vertices1[indices1[i+2]].normal,
				t*ray.direction + ray.origin);
			break;
		}
		i = segment2.indexStart + offset;
		p1 = vertices2[indices2[i+0]].position;
		p2 = vertices2[indices2[i+1]].position;
		p3 = vertices2[indices2[i+2]].position;
		t = intersectsFrontTriangle(ray, p1, p2, p3);
		if (t != 0.0f) {
			vertex.normal = getSurfaceNormal(p1, p2, p3,
				vertices2[indices2[i+0]].normal,
				vertices2[indices2[i+1]].normal,
				vertices2[indices2[i+2]].normal,
				t*ray.direction + ray.origin);
			break;
		}
	}

	if (t != 0.0f) {
		vertex.normal = normalize(vertex.normal);
		vertex.position = ray.origin + t * ray.direction;
		return vertex;
	} else {
		vertex.position = Vec3(std::numeric_limits<float>::infinity());
		return vertex;
	}
}

/** Interpolate normals from the first cross section of the stem with normals
from the first cross section after the branch collar. */
void Collar::setBranchCollarNormals(size_t index1, size_t index2, int mesh,
	int sdivisions, int cdivisions)
{
	DVertex *buffer1 = &this->mesh.vertices[mesh][0];
	DVertex *buffer2 = &this->mesh.vertices[mesh][0];
	for (int i = 0; i < sdivisions; i++) {
		Vec3 normal1 = buffer1[index1].normal;
		Vec3 normal2 = buffer2[index2].normal;
		Vec3 tangent1 = buffer1[index1].tangent;
		Vec3 tangent2 = buffer2[index2].tangent;

		for (int j = 1; j <= cdivisions; j++) {
			float t = j/static_cast<float>(cdivisions);
			float a = 2.0f*t*t;
			float b = -2.0f*t*t + 4.0f*t - 1.0f;
			t = a * (t <= 0.5f) + b * (t > 0.5f);

			Vec3 normal = lerp(normal1, normal2, t);
			normal = normalize(normal);
			Vec3 tangent = lerp(tangent1, tangent2, t);
			tangent = normalize(tangent);

			size_t offset = j * sdivisions;
			buffer1[index1 + offset].normal = normal;
			buffer1[index1 + offset].tangent = tangent;
		}

		index1++;
		index2++;
	}
}

/** Normally UV coordinates are generated starting at the first cross section.
The UV coordinates for branch collars are generated backwards because splines
are not guaranteed to be the same length. */
void Collar::setBranchCollarUVs(size_t lastIndex, Stem *stem, int mesh,
	int sdivisions, int cdivisions)
{
	DVertex *buffer = &this->mesh.vertices[mesh][lastIndex];
	float radius = this->plant->getRadius(stem, 1);
	float aspect = getAspect(this->plant, stem);

	for (int i = 0; i < sdivisions; i++) {
		Vec2 uv = buffer[i].uv;
		size_t index = i;

		for (int j = cdivisions; j >= 0; j--) {
			Vec3 p1 = buffer[index].position;
			index -= sdivisions;
			Vec3 p2 = buffer[index].position;
			float length = magnitude(p2 - p1);
			uv.y -= (length * aspect) / (radius * 2.0f * pi);
			buffer[index].uv = uv;
		}
	}
}

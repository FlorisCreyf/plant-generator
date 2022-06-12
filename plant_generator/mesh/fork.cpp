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

#include "fork.h"
#include <map>
#include <vector>

const float pi = 3.14159265359f;

using namespace pg;
using std::vector;

Fork::Fork(Mesh &mesh) : mesh(mesh)
{

}

bool Fork::isValidFork(Stem *stem, Stem *fork[2])
{
	if (fork[0] && fork[1]) {
		const Path &p0 = stem->getPath();
		const Path &p1 = fork[0]->getPath();
		const Path &p2 = fork[1]->getPath();
		size_t d = p0.getInitialDivisions() + 3;
		if (p0.getSize() < d)
			return false;
		if (p1.getSize() < 2 || p2.getSize() < 2)
			return false;
		if (p1.get(0) == p1.get(1) || p2.get(0) == p2.get(1))
			return false;
		int d1 = stem->getSectionDivisions();
		int d2 = fork[0]->getSectionDivisions();
		int d3 = fork[1]->getSectionDivisions();
		if (d2 != d3 || d1 != d2 || d2 % 2 != 0)
			return false;
		int m1 = fork[0]->getMaterial(Stem::Outer);
		int m2 = fork[1]->getMaterial(Stem::Outer);
		int i1 = fork[0]->getPath().getInitialDivisions();
		int i2 = fork[1]->getPath().getInitialDivisions();
		if (m1 != m2 || i1 != i2)
			return false;
		return true;
	}
	return false;
}

size_t Fork::getForkVertexCount(const Stem *stem)
{
	int divisions = stem->getPath().getInitialDivisions();
	divisions -= (divisions > 0);
	return (stem->getSectionDivisions() + 1) * divisions;
}

size_t Fork::getForkIndexCount(const Stem *stem)
{
	int divisions = stem->getPath().getInitialDivisions();
	return stem->getSectionDivisions() * divisions * 6;
}

void Fork::reserveForkSpace(const Stem *stem, int mesh)
{
	size_t fsize = getForkVertexCount(stem);
	size_t msize = this->mesh.vertices[mesh].size();
	this->mesh.vertices[mesh].resize(msize + fsize);
	fsize = getForkIndexCount(stem);
	msize = this->mesh.indices[mesh].size();
	this->mesh.indices[mesh].resize(msize + fsize);
}

void Fork::setBaseUVs(Stem *stem, int sdivisions, int cdivisions, DVertex *v)
{
	const Vec3 location = stem->getLocation();
	const Path &path = stem->getPath();
	const Vec3 c[2] = {
		location + path.get(path.getSize()-path.getDivisions()-2),
		location + path.get(path.getSize()-1)
	};
	Vec3 direction = normalize(c[1]-c[0]);
	for (int i = 0, offset = 0; i < cdivisions; i++) {
		for (int j = 0; j < sdivisions; j++, offset++) {
			float y = dot(v[offset].position - c[0], direction);
			v[offset].uv.y = v[j-sdivisions].uv.y + y;
			v[offset].uv.x = v[j-sdivisions].uv.x;
		}
	}
}

void Fork::reserveExtraTriangles(Section middle, Mesh::State &state)
{
	if (middle.offset == 0) {
		size_t size = this->mesh.indices[state.mesh].size() + 6;
		this->mesh.indices[state.mesh].resize(size);
		state.segment.indexCount += 6;
	}
}

void Fork::addExtraTriangles(vector<unsigned> &indices, Section &middle,
	int sd, size_t start1, size_t start2, Mesh::Segment &segment)
{
	if (middle.offset == 0) {
		segment.indexCount -= 6;

		size_t start = segment.vertexStart;
		size_t count = segment.vertexCount;
		size_t start0 = start + count - sd - 1;
		int offset = segment.indexStart + segment.indexCount + 6;
		int p = middle.edge1 - 1;
		p += sd * (p < 0);
		middle.edge1 += sd * (middle.edge1 == 0);
		indices[offset-6] = start0 + middle.edge1;
		indices[offset-5] = start2 + middle.edge1;
		indices[offset-4] = start0 + p;
		middle.edge2 += sd * (middle.edge2 <= 0);
		p = middle.edge2 - 1;
		p += sd * (p < 0);
		indices[offset-3] = start0 + middle.edge2;
		indices[offset-2] = start1 + middle.edge2;
		indices[offset-1] = start0 + p;
	}
}

void insertCurve(int sdivisions, int cdivisions, DVertex *v1, DVertex *v2)
{
	Vec3 curve[2];
	Vec3 normal[2];
	Vec3 tangent[2];
	Vec2 weight[2];
	Vec2 joints[2];

	int b = sdivisions * cdivisions;
	curve[0] = v2[0].position;
	curve[1] = v1[b].position;
	normal[0] = v2[0].normal;
	normal[1] = v1[b].normal;
	tangent[0] = v2[0].tangent;
	tangent[1] = v1[b].tangent;
	weight[0] = v1[b].weights;
	weight[1] = v1[b].weights;
	joints[0] = v1[b].indices;
	joints[1] = v1[b].indices;

	float t = 0.0f;
	float dt = 1.0f / cdivisions;
	size_t index = 0;
	for (int i = 0; i < cdivisions; i++) {
		v1[index].position = getLinearBezier(t, curve[0], curve[1]);
		v1[index].normal = normalize(lerp(normal[0], normal[1], t));
		v1[index].tangent = normalize(lerp(tangent[0], tangent[1], t));
		v1[index].weights = lerp(weight[0], weight[1], t);
		v1[index].indices = joints[0];
		index += sdivisions;
		t += dt;
	}
}

void insertCurve(int sdivisions, int cdivisions, int size, bool invert,
	DVertex *v1, DVertex *v2)
{
	Vec3 curve[3];
	Vec3 normal[2];
	Vec3 tangent[2];
	Vec2 weight[2];
	Vec2 joints[2];
	size_t index = 0;
	float ds = 1.0f / cdivisions;
	float dt = 0.5f / cdivisions;
	float s, t;

	if (invert) {
		t = 0.5f;
		s = 0.0f;
		dt = -dt;
		const int i = sdivisions + size;
		curve[0] = v1[i].position;
		curve[1] = v1[0].position;
		normal[0] = v1[i].normal;
		tangent[0] = v1[i].tangent;
		weight[0] = v1[0].weights;
		weight[1] = v1[i].weights;
		joints[0] = v1[0].indices;
		joints[1] = v1[i].indices;
	} else {
		s = ds;
		t = dt;
		const int i = -sdivisions;
		curve[0] = v1[i].position;
		curve[1] = v1[size].position;
		normal[0] = v1[i].normal;
		tangent[0] = v1[i].tangent;
		weight[0] = v1[i].weights;
		weight[1] = v1[size].weights;
		joints[0] = v1[i].indices;
		joints[1] = v1[size].indices;
	}
	curve[2] = v2[sdivisions+size].position;
	normal[1] = v2[sdivisions+size].normal;
	tangent[1] = v2[sdivisions+size].tangent;

	for (int j = 0; j < cdivisions; j++) {
		Vec3 p = getQuadraticBezier(t, curve[0], curve[1], curve[2]);
		v1[index].position = p;
		v1[index].normal = normalize(lerp(normal[0], normal[1], t));
		v1[index].tangent = normalize(lerp(tangent[0], tangent[1], t));
		v1[index].weights = lerp(weight[0], weight[1], s);
		v1[index].indices = joints[0];
		index += sdivisions;
		t += dt;
		s += ds;
	}

	t = 0.5f;
	s = 0.0f;
	index = 0;
	weight[0] = v2[0].weights;
	weight[1] = v2[sdivisions+size].weights;
	joints[0] = v2[0].indices;
	joints[1] = v2[sdivisions+size].indices;
	if (invert)
		dt = -dt;

	for (int j = 0; j < cdivisions; j++) {
		Vec3 p = getQuadraticBezier(t, curve[0], curve[1], curve[2]);
		v2[index].position = p;
		v2[index].normal = normalize(lerp(normal[0], normal[1], t));
		v2[index].tangent = normalize(lerp(tangent[0], tangent[1], t));
		v2[index].weights = lerp(weight[0], weight[1], s);
		v2[index].indices = joints[1];
		index += sdivisions;
		t += dt;
		s += ds;
	}
}

Fork::Section Fork::getMiddle(Stem *fork[2], Mesh::State &state)
{
	const int cd = fork[0]->getPath().getInitialDivisions();
	const int sd = state.segment.stem->getSectionDivisions();
	const Vec3 direction1 = normalize(fork[0]->getPath().get(cd+1));
	const Vec3 direction2 = normalize(fork[1]->getPath().get(cd+1));
	const int quad = sd % 4 == 0;

	Section middle;
	middle.length = sd / 2;
	int midpoint = getForkMidpoint(2*sd, direction1, direction2,
		state.prevDirection, state.prevRotation);

	if ((quad && midpoint % 2 == 0) || (!quad && midpoint % 2 != 0)) {
		midpoint /= 2;
		middle.offset = 1;
		middle.edge1 = midpoint - middle.length / 2;
		middle.edge2 = middle.edge1 + middle.length;
		middle.edge1 += (middle.edge1 < 0) * sd;
		middle.edge2 -= (middle.edge2 >= sd) * sd;
	} else {
		midpoint /= 2;
		middle.offset = 0;
		middle.edge1 = midpoint - middle.length / 2 + quad;
		middle.edge2 = midpoint + middle.length / 2 + 1;
		middle.edge1 += (middle.edge1 < 0) * sd;
		middle.edge2 -= (middle.edge2 >= sd) * sd;
		middle.length -= 1;
	}

	return middle;
}

void Fork::connectForks(Stem *fork[2], Mesh::Segment segments[2],
	Mesh::State fs[2], Mesh::State &state, Section mid)
{
	const int cd = fork[0]->getPath().getInitialDivisions();
	const int sd = state.segment.stem->getSectionDivisions();
	const Vec3 direction1 = normalize(fork[0]->getPath().get(cd+1));
	const Vec3 direction2 = normalize(fork[1]->getPath().get(cd+1));

	addExtraTriangles(this->mesh.indices[state.mesh], mid, sd,
		segments[0].vertexStart, segments[1].vertexStart,
		state.segment);

	const size_t size = getForkVertexCount(fork[0]);
	DVertex *v1 = &this->mesh.vertices[fs[0].mesh][segments[0].vertexStart];
	DVertex *v2 = &this->mesh.vertices[fs[1].mesh][segments[1].vertexStart];
	DVertex *v0 = &this->mesh.vertices[state.mesh][
		state.segment.vertexStart+state.segment.vertexCount-sd-1];

	Ray ray;
	Plane plane1;
	plane1.point = fork[0]->getLocation();
	plane1.normal = normalize(state.prevDirection+direction1);
	Plane plane2;
	plane2.point = fork[1]->getLocation();
	plane2.normal = normalize(state.prevDirection+direction2);
	Plane plane3;
	plane3.point = plane1.point;
	{
		Vec3 n1 = cross(direction2, direction1);
		Vec3 n2 = normalize(cross(n1, direction2));
		Vec3 n3 = normalize(cross(n1, direction1));
		plane3.normal = normalize(n2 + n3);
	}

	for (int i = 0; i <= mid.length; i++) {
		int k = mid.edge1 + i;
		k -= (k >= sd) * sd;
		ray.origin = v0[k].position;
		ray.direction = state.prevDirection;
		float t = intersectsPlane(ray, plane1);
		v0[k].position = ray.origin + t*ray.direction;
		v1[k].position = v0[k].position;
		v0[k].normal = normalize(v0[k].normal + v1[k].normal);
		v1[k].normal = v0[k].normal;
		v0[k].tangent = normalize(v0[k].tangent + v1[k].tangent);
		v1[k].tangent = v0[k].tangent;
		v0[k].uv.y += t;
		v1[k].uv.y += v0[k].uv.y;
		insertCurve(sd+1, cd, size, false, &v0[k-size], &v1[k]);
	}
	for (int i = mid.offset; i <= mid.length - mid.offset; i++) {
		int k = mid.edge2 + i;
		k -= (k >= sd) * sd;
		ray.origin = v0[k].position;
		ray.direction = state.prevDirection;
		float t = intersectsPlane(ray, plane2);
		v0[k].position = ray.origin + t*ray.direction;
		v2[k].position = v0[k].position;
		v0[k].normal = normalize(v0[k].normal + v2[k].normal);
		v2[k].normal = v0[k].normal;
		v0[k].tangent = normalize(v0[k].tangent + v2[k].tangent);
		v2[k].tangent = v0[k].tangent;
		v0[k].uv.y += t;
		v2[k].uv.y += v0[k].uv.y;
		insertCurve(sd+1, cd, size, false, &v0[k-size], &v2[k]);
	}
	if (mid.offset > 0) {
		v2[mid.edge2].position = v1[mid.edge2].position;
		v2[mid.edge1].position = v1[mid.edge1].position;
		v2[mid.edge2].normal = v1[mid.edge2].normal;
		v2[mid.edge1].normal = v1[mid.edge1].normal;
		v2[mid.edge2].tangent = v1[mid.edge2].tangent;
		v2[mid.edge1].tangent = v1[mid.edge1].tangent;
	}
	for (int i = mid.offset; i <= mid.length - mid.offset; i++) {
		int k1 = mid.edge1 - 1 + mid.offset - i;
		int k2 = mid.edge1 + i;
		k1 += (k1 < 0) * sd;
		k1 -= (k1 >= sd) * sd;
		k2 += (k2 < 0) * sd;
		k2 -= (k2 >= sd) * sd;
		ray.origin = v1[k1].position;
		ray.direction = direction1;
		float t = intersectsPlane(ray, plane3);
		v1[k1].position = ray.origin + t*ray.direction;
		v2[k2].position = v1[k1].position;
		v1[k1].normal = normalize(v1[k1].normal + v2[k2].normal);
		v2[k2].normal = v1[k1].normal;
		v1[k1].tangent = normalize(v1[k1].tangent + v2[k2].tangent);
		v2[k2].tangent = v1[k1].tangent;
		v1[k1].uv.y += t;
		v2[k2].uv.y += t;
		insertCurve(sd+1, cd, size, true, &v1[k1], &v2[k2]);
	}
	if (mid.offset == 1 && cd > 1) {
		insertCurve(sd+1, cd, &v2[mid.edge2], &v0[mid.edge2]);
		insertCurve(sd+1, cd, &v2[mid.edge1], &v0[mid.edge1]);
	}
	for (int i = 0; i < cd || i == 0; i++) {
		size_t a = i*(sd+1);
		size_t b = a + sd;
		v0[b-size].position = v0[a-size].position;
		v0[b-size].normal = v0[a-size].normal;
		v0[b-size].tangent = v0[a-size].tangent;
		v0[b-size].weights = v0[a-size].weights;
		v0[b-size].indices = v0[a-size].indices;
		v0[b-size].uv.y = v0[a-size].uv.y;
		v0[b-size].uv.x = 0.0f;
		v1[b].position = v1[a].position;
		v1[b].normal = v1[a].normal;
		v1[b].tangent = v1[a].tangent;
		v1[b].weights = v1[a].weights;
		v1[b].indices = v1[a].indices;
		v1[b].uv.y = v1[a].uv.y;
		v1[b].uv.x = 0.0f;
		v2[b].position = v2[a].position;
		v2[b].normal = v2[a].normal;
		v2[b].tangent = v2[a].tangent;
		v2[b].weights = v2[a].weights;
		v2[b].indices = v2[a].indices;
		v2[b].uv.y = v2[a].uv.y;
		v2[b].uv.x = 0.0f;
	}
	if (cd > 0) {
		setBaseUVs(state.segment.stem, sd+1, cd, &v0[-size]);
		setForkUVs(fork[0], sd+1, cd, &v1[0]);
		setForkUVs(fork[1], sd+1, cd, &v2[0]);
		addForkTriangles(state, fs, fork, segments);
	}
	if (mid.offset == 0)
		state.segment.indexCount += 6;
}

void Fork::addForkTriangles(const Mesh::State &state, const Mesh::State fs[2],
	Stem *fork[2], const Mesh::Segment segments[2])
{
	const int cdivisions = fork[0]->getPath().getInitialDivisions();
	const int sdivisions = state.segment.stem->getSectionDivisions() + 1;

	unsigned *indices = &this->mesh.indices[state.mesh][
		state.segment.indexStart + state.segment.indexCount -
		getForkIndexCount(fork[0])];
	size_t index1 =
		state.segment.vertexStart + state.segment.vertexCount -
		getForkVertexCount(fork[0]) - sdivisions*2;
	size_t index2 = index1 + sdivisions;
	for (int i = 0; i < cdivisions; i++) {
		indices += this->mesh.insertTriangleRing(
			index2, index1, sdivisions-1, indices);
		index1 = index2;
		index2 += sdivisions;
	}

	indices = &this->mesh.indices[fs[0].mesh][segments[0].indexStart];
	index1 = segments[0].vertexStart;
	index2 = index1 + sdivisions;
	for (int i = 0; i < cdivisions; i++) {
		indices += this->mesh.insertTriangleRing(
			index2, index1, sdivisions-1, indices);
		index1 = index2;
		index2 += sdivisions;
	}

	indices = &this->mesh.indices[fs[1].mesh][segments[1].indexStart];
	index1 = segments[1].vertexStart;
	index2 = index1 + sdivisions;
	for (int i = 0; i < cdivisions; i++) {
		indices += this->mesh.insertTriangleRing(
			index2, index1, sdivisions-1, indices);
		index1 = index2;
		index2 += sdivisions;
	}
}

/** Return the point index that is between the dividing line of the section. */
int Fork::getForkMidpoint(int divisions, Vec3 direction1, Vec3 direction2,
	Vec3 direction, Quat rotation)
{
	/* Average all vectors and select one of three frames of reference.
	Nothing is swapped if the child stems point in the same direction as
	the parent stem. */
	Vec3 d = normalize(-direction + direction1 + direction2);
	float d1 = dot(-direction, d);
	float d2 = dot(direction1, d);
	float d3 = dot(direction2, d);
	if (d2 < d1 && d2 < d3) {
		rotation = rotateIntoVecQ(direction, direction1) * rotation;
		Vec3 swap = -direction;
		direction = -direction1;
		direction1 = swap;
	} else if (d3 < d1 && d3 < d2) {
		rotation = rotateIntoVecQ(direction, direction2) * rotation;
		Vec3 swap = -direction;
		direction = -direction2;
		direction2 = swap;
	}

	/* Project a stem direction onto the cross section and measure the
	angle between that projected vector and the first point of the cross
	section. */
	Vec3 point = rotate(rotation, Vec3(1.0f, 0.0f, 0.0f));
	Vec3 avg = normalize(direction1 + direction2);
	Quat sectionRotation = rotateIntoVecQ(avg, direction);
	direction1 = rotate(sectionRotation, direction1);
	Vec3 projection1 = normalize(projectOntoPlane(direction1, direction));
	Vec3 projection2 = cross(projection1, direction);
	if (d1 > d2 || d1 > d3)
		projection2 = -projection2;

	float x = dot(point, projection1);
	if (x > 1.0f)
		x = 1.0f;
	if (x < -1.0f)
		x = -1.0f;
	float delta = 2.0f * pi / divisions;
	float theta = std::acos(x);
	if (dot(point, projection2) < 0.0f)
		theta = 2.0f * pi - theta;

	int midpoint;
	if (divisions % 4 == 0)
		midpoint = std::round(theta / delta);
	else
		midpoint = std::floor(theta / delta);
	midpoint -= (midpoint >= divisions) * divisions;
	return midpoint;
}

void Fork::setForkUVs(Stem *stem, int sdivisions, int cdivisions, DVertex *v)
{
	const int size = getForkVertexCount(stem) + sdivisions;
	const Path &path = stem->getPath();
	const Vec3 c[2] = {
		stem->getLocation(),
		stem->getLocation() + path.get(path.getInitialDivisions()+1)
	};
	Vec3 direction = normalize(c[1]-c[0]);
	for (int i = 0, offset = 0; i < cdivisions; i++) {
		for (int j = 0; j < sdivisions; j++, offset++) {
			float y = dot(c[1] - v[offset].position, direction);
			v[offset].uv.y = v[size+j].uv.y - y;
			v[offset].uv.x = v[size+j].uv.x;
		}
	}
}

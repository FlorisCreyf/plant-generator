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

#include "mesh.h"
#include <cmath>
#include <limits>

using namespace pg;
using std::map;
using std::pair;
using std::vector;

const float pi = 3.14159265359f;

Mesh::Mesh(Plant *plant) : plant(plant)
{

}

void Mesh::generate()
{
	Stem *stem = this->plant->getRoot();
	initBuffer();
	if (stem) {
		State parentState = {};
		State state;
		state.prevRotation = Quat(0.0f, 0.0f, 0.0f, 1.0f);
		state.prevDirection = Vec3(0.0f, 1.0f, 0.0f);
		addStem(stem, state, parentState, false);
		updateSegments();
	}
}

bool isValidFork(Stem *stem, Stem *fork[2])
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

Segment Mesh::addStem(Stem *stem, State &state, State parentState, bool isFork)
{
	Stem *fork[2];
	stem->getFork(fork);
	if (!isValidFork(stem, fork))
		fork[0] = fork[1] = nullptr;

	state.mesh = stem->getMaterial(Stem::Outer);
	state.segment.stem = stem;
	state.segment.vertexStart = this->vertices[state.mesh].size();
	state.segment.indexStart = this->indices[state.mesh].size();
	setInitialJointState(state, parentState);
	addSections(state, parentState.segment, isFork, fork[0]);
	state.segment.vertexCount = this->vertices[state.mesh].size();
	state.segment.vertexCount -= state.segment.vertexStart;
	state.segment.indexCount = this->indices[state.mesh].size();
	state.segment.indexCount -= state.segment.indexStart;
	this->stemSegments[state.mesh].emplace(stem, state.segment);
	addLeaves(stem, state);

	/* The parent stem finishes generating both forks and will generate the
	child stems for this stem. */
	if (!isFork)
		addChildStems(stem, fork, state);
	return state.segment;
}

void Mesh::addChildStems(Stem *stem, Stem *fork[2], State &state)
{
	if (fork[0])
		if (!addForks(fork, state))
			fork[0] = fork[1] = nullptr;

	Stem *child = stem->getChild();
	while (child) {
		if (fork[0] != child && fork[1] != child) {
			State childState;
			setInitialRotation(child, childState);
			addStem(child, childState, state, false);
		}
		child = child->getSibling();
	}
}

size_t getSectionCount(Stem *stem, Stem *fork)
{
	const Path &path = stem->getPath();
	if (fork)
		return path.getSize() - path.getDivisions() - 1;
	else
		return path.getSize();
}

void Mesh::addSections(State &state, Segment parentSegment,
	bool isFork, Stem *fork)
{
	Stem *stem = state.segment.stem;
	state.prevIndex = this->vertices[state.mesh].size();
	if (stem->getSectionDivisions() != this->crossSection.getResolution())
		this->crossSection.generate(stem->getSectionDivisions());
	if (isFork)
		createFork(stem, state);
	else {
		state.section = 0;
		state.texOffset = 0.0f;
		Vec2 swelling = stem->getSwelling();
		if (swelling.x >= 1.0f && swelling.y >= 1.0f) {
			createBranchCollar(state, parentSegment);
			connectCollar(state, fork);
		}
	}

	size_t sections = getSectionCount(stem, fork);
	for (; state.section < sections; state.section++) {
		Quat rotation = rotateSection(state);
		state.prevIndex = this->vertices[state.mesh].size();
		addSection(state, rotation, this->crossSection);
		if (state.section+1 < sections)
			addTriangleRing(
				state.prevIndex,
				this->vertices[state.mesh].size(),
				stem->getSectionDivisions(),
				state.mesh);
	}

	if (fork) {
		if (fork->getPath().getInitialDivisions() == 0)
			addTriangleRing(
				state.prevIndex,
				this->vertices[state.mesh].size(),
				stem->getSectionDivisions(),
				state.mesh);
		else
			reserveForkSpace(fork, state.mesh);

		Quat rotation = rotateSection(state);
		state.prevIndex = this->vertices[state.mesh].size();
		size_t section1 = state.section;
		size_t section2 = stem->getPath().getSize() - 1;
		state.texOffset += getTextureLength(stem, section1, section2);
		state.section = section2;
		addSection(state, rotation, this->crossSection);
	} else if (!fork && stem->getMinRadius() > 0)
		capStem(stem, state.mesh, state.prevIndex);
}

/** Generate a cross section for a point in the stem's path. Indices are added
at a later stage to connect the sections. */
void Mesh::addSection(State &state, Quat rotation, const CrossSection &section)
{
	Stem *stem = state.segment.stem;
	size_t index = state.section;
	DVertex vertex;
	vertex.tangentScale = 1.0f;
	vertex.tangent = stem->getPath().getAverageDirection(index);
	vertex.uv.y = getTextureLength(stem, index) + state.texOffset;
	state.texOffset = vertex.uv.y;
	Vec3 location = stem->getLocation();
	location += stem->getPath().get(index);

	Vec2 indices;
	Vec2 weights;
	if (stem->getJoints().size() > 0)
		updateJointState(state, indices, weights);
	else {
		indices = Vec2(state.jointID, 0.0f);
		weights = Vec2(1.0f, 0.0f);
	}

	float radius = this->plant->getRadius(stem, index);
	const std::vector<SVertex> sectionVertices = section.getVertices();
	for (size_t i = 0; i < sectionVertices.size(); i++) {
		vertex.position = sectionVertices[i].position;
		vertex.position = radius * vertex.position;
		vertex.position = rotate(rotation, vertex.position);
		vertex.position += location;
		vertex.normal = sectionVertices[i].normal;
		vertex.normal = rotate(rotation, vertex.normal);
		vertex.normal = normalize(vertex.normal);
		vertex.uv.x = sectionVertices[i].uv.x;
		vertex.weights = weights;
		vertex.indices = indices;
		this->vertices[state.mesh].push_back(vertex);
	}
}

/** The cross section is rotated so that the first point is always the topmost
point relative to the parent stem direction. */
void Mesh::setInitialRotation(Stem *stem, State &state)
{
	float position = stem->getDistance();
	const Path &parentPath = stem->getParent()->getPath();
	Vec3 parentDirection;
	parentDirection = parentPath.getIntermediateDirection(position);
	const Path &path = stem->getPath();
	Vec3 stemDirection = path.getDirection(0);
	Vec3 up(0.0f, 1.0f, 0.0f);
	state.prevRotation = rotateIntoVecQ(up, stemDirection);
	state.prevDirection = stemDirection;

	Vec3 sideways(1.0f, 0.0f, 0.0f);
	sideways = rotate(state.prevRotation, sideways);
	sideways = normalize(sideways);
	up = projectOntoPlane(parentDirection, stemDirection);
	up = normalize(up);
	Quat rotation = rotateIntoVecQ(sideways, up);
	state.prevRotation = rotation * state.prevRotation;
}

/** A rotation for a cross section is relative to the rotation of the previous
cross section. Twisting can occur in stems if all sections are rotated relative
to the global axis. */
Quat Mesh::rotateSection(State &state)
{
	Stem *stem = state.segment.stem;
	const Path &path = stem->getPath();
	Vec3 direction = path.getAverageDirection(state.section);
	Quat rotation = rotateIntoVecQ(state.prevDirection, direction);
	rotation *= state.prevRotation;
	state.prevRotation = rotation;
	state.prevDirection = direction;
	return rotation;
}

float getAspect(const Plant *plant, const Stem *stem)
{
	float aspect = 1.0f;
	long materialID = stem->getMaterial(Stem::Outer);
	if (materialID > 0) {
		Material m = plant->getMaterial(materialID);
		aspect = m.getRatio();
	}
	return aspect;
}

/** Determine a length to preserve the aspect ratio throughout the stem. */
float Mesh::getTextureLength(Stem *stem, size_t section)
{
	if (section > 0) {
		float length = stem->getPath().getSegmentLength(section);
		float radius = this->plant->getRadius(stem, section - 1);
		float aspect = getAspect(this->plant, stem);
		return (length * aspect) / (radius * 2.0f * pi);
	} else
		return 0.0f;
}

float Mesh::getTextureLength(Stem *stem, size_t section1, size_t section2)
{
	Vec3 p1 = stem->getPath().get(section1);
	Vec3 p2 = stem->getPath().get(section2);
	float length = magnitude(p2 - p1);
	float radius = this->plant->getRadius(stem, section1);
	float aspect = getAspect(this->plant, stem);
	return (length * aspect) / (radius * 2.0f * pi);
}

inline size_t getForkVertexCount(const Stem *stem)
{
	int divisions = stem->getPath().getInitialDivisions();
	divisions -= (divisions > 0);
	return (stem->getSectionDivisions() + 1) * divisions;
}

inline size_t getForkIndexCount(const Stem *stem)
{
	int divisions = stem->getPath().getInitialDivisions();
	return stem->getSectionDivisions() * divisions * 6;
}

void Mesh::reserveForkSpace(const Stem *stem, int mesh)
{
	size_t size = getForkVertexCount(stem) + this->vertices[mesh].size();
	this->vertices[mesh].resize(size);
	size = getForkIndexCount(stem) + this->indices[mesh].size();
	this->indices[mesh].resize(size);
}

void Mesh::createFork(Stem *stem, State &state)
{
	Quat rotation = state.prevRotation;
	size_t section = state.section;
	state.section = 0;
	addSection(state, rotation, this->crossSection);
	state.section = section;
	state.texOffset += getTextureLength(stem, 0, section - 1);

	if (stem->getPath().getInitialDivisions() == 0)
		addTriangleRing(state.prevIndex,
			this->vertices[state.mesh].size(),
			stem->getSectionDivisions(), state.mesh);
	else
		reserveForkSpace(stem, state.mesh);
}

inline void addExtraTriangles(vector<unsigned> &indices, int edge1, int edge2,
	int divisions, size_t start0, size_t start1, size_t start2)
{
	int p = edge1 - 1;
	p += divisions * (p < 0);
	indices.push_back(start2 + edge1);
	indices.push_back(start0 + edge1);
	indices.push_back(start0 + p);
	edge2 += divisions * (edge2 <= 0);
	p = edge2 - 1;
	p += divisions * (p < 0);
	indices.push_back(start0 + edge2);
	indices.push_back(start0 + p);
	indices.push_back(start1 + edge2);
}

void insertCurve(int sectionDivisions, int collarDivisions, int size,
	DVertex *v1, DVertex *v2)
{
	Vec3 curve[2];
	Vec3 normal[2];
	Vec3 tangent[2];
	Vec2 weight[2];
	Vec2 joints[2];
	{
		int a = -size+collarDivisions*sectionDivisions;
		int b = size+sectionDivisions;
		curve[0] = v2[a].position;
		curve[1] = v1[b].position;
		normal[0] = v2[a].normal;
		normal[1] = v1[b].normal;
		tangent[0] = v2[a].tangent;
		tangent[1] = v1[b].tangent;
		weight[0] = v1[b].weights;
		weight[1] = v1[b].weights;
		joints[0] = v1[b].indices;
		joints[1] = v1[b].indices;
	}

	float dt = 1.0f / collarDivisions;
	float t = 0.0f;
	size_t index = 0;
	for (int i = 0; i < collarDivisions; i++) {
		v1[index].position = getLinearBezier(t, curve[0], curve[1]);
		v1[index].normal = normalize(lerp(normal[0], normal[1], t));
		v1[index].tangent = normalize(lerp(tangent[0], tangent[1], t));
		v1[index].weights = lerp(weight[0], weight[1], t);
		v1[index].indices = joints[0];
		index += sectionDivisions;
		t += dt;
	}
}

void insertCurve(int sectionDivisions, int collarDivisions, int size,
	bool invert, DVertex *v1, DVertex *v2)
{
	Vec3 curve[3];
	Vec3 normal[2];
	Vec3 tangent[2];
	Vec2 weight[2];
	Vec2 joints[2];
	size_t index = 0;
	float ds = 1.0f / collarDivisions;
	float dt = 0.5f / collarDivisions;
	float s;
	float t;

	if (invert) {
		t = 0.5f;
		s = 0.0f;
		dt = -dt;
		const int i = sectionDivisions + size;
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
		const int i = -sectionDivisions;
		curve[0] = v1[i].position;
		curve[1] = v1[size].position;
		normal[0] = v1[i].normal;
		tangent[0] = v1[i].tangent;
		weight[0] = v1[i].weights;
		weight[1] = v1[size].weights;
		joints[0] = v1[i].indices;
		joints[1] = v1[size].indices;
	}
	curve[2] = v2[sectionDivisions+size].position;
	normal[1] = v2[sectionDivisions+size].normal;
	tangent[1] = v2[sectionDivisions+size].tangent;

	for (int j = 0; j < collarDivisions; j++) {
		Vec3 p = getQuadraticBezier(t, curve[0], curve[1], curve[2]);
		v1[index].position = p;
		v1[index].normal = normalize(lerp(normal[0], normal[1], t));
		v1[index].tangent = normalize(lerp(tangent[0], tangent[1], t));
		v1[index].weights = lerp(weight[0], weight[1], s);
		v1[index].indices = joints[0];
		index += sectionDivisions;
		t += dt;
		s += ds;
	}

	t = 0.5f;
	s = 0.0f;
	index = 0;
	weight[0] = v2[0].weights;
	weight[1] = v2[sectionDivisions+size].weights;
	joints[0] = v2[0].indices;
	joints[1] = v2[sectionDivisions+size].indices;
	if (invert)
		dt = -dt;

	for (int j = 0; j < collarDivisions; j++) {
		Vec3 p = getQuadraticBezier(t, curve[0], curve[1], curve[2]);
		v2[index].position = p;
		v2[index].normal = normalize(lerp(normal[0], normal[1], t));
		v2[index].tangent = normalize(lerp(tangent[0], tangent[1], t));
		v2[index].weights = lerp(weight[0], weight[1], s);
		v2[index].indices = joints[1];
		index += sectionDivisions;
		t += dt;
		s += ds;
	}
}

void setFork1UVs(Stem *stem, int sDivisions, int cDivisions, DVertex *v)
{
	const Vec3 location = stem->getLocation();
	const Path &path = stem->getPath();
	const Vec3 c[2] = {
		location + path.get(path.getSize()-path.getDivisions()-2),
		location + path.get(path.getSize()-1)};
	for (int i = 0, offset = 0; i < cDivisions; i++)
		for (int j = 0; j < sDivisions; j++, offset++) {
			Vec3 p = v[offset].position;
			float y = project(p-c[0], normalize(c[1]-c[0]));
			v[offset].uv.y = v[j-sDivisions].uv.y + y;
			v[offset].uv.x = v[j-sDivisions].uv.x;
		}
}

void setFork2UVs(Stem *stem, int sDivisions, int cDivisions, DVertex *v)
{
	const int size = getForkVertexCount(stem) + sDivisions;
	const Path &path = stem->getPath();
	const Vec3 c[2] = {
		stem->getLocation() + path.get(0),
		stem->getLocation() + path.get(path.getInitialDivisions()+1)};
	for (int i = 0, offset = 0; i < cDivisions; i++)
		for (int j = 0; j < sDivisions; j++, offset++) {
			Vec3 p = v[offset].position;
			float y = project(c[1]-p, normalize(c[1]-c[0]));
			v[offset].uv.y = v[size+j].uv.y - y;
			v[offset].uv.x = v[size+j].uv.x;
		}
}

bool Mesh::addForks(Stem *fork[2], State state)
{
	const int cDivisions = fork[0]->getPath().getInitialDivisions();
	const int sDivisions = state.segment.stem->getSectionDivisions();
	/** The directions from the fork origin to the fork boundary. */
	const Vec3 direction1 = normalize(fork[0]->getPath().get(cDivisions+1));
	const Vec3 direction2 = normalize(fork[1]->getPath().get(cDivisions+1));
	const size_t size = getForkVertexCount(fork[0]);

	/** Generate stems. The first cross section is followed with enough
	empty space to fill with curves. Points of the first cross sections are
	used to compute the tangents for the curves and are overwritten. */
	Segment segments[2];
	State fs[2];
	fs[0].mesh = fork[0]->getMaterial(Stem::Outer);
	fs[0].section = cDivisions + 1;
	fs[0].texOffset = state.texOffset;
	fs[0].prevRotation = rotateIntoVecQ(state.prevDirection, direction1);
	fs[0].prevRotation *= state.prevRotation;
	fs[0].prevDirection = direction1;
	segments[0] = addStem(fork[0], fs[0], state, true);
	fs[1].mesh = fork[1]->getMaterial(Stem::Outer);
	fs[1].section = cDivisions + 1;
	fs[1].texOffset = state.texOffset;
	fs[1].prevRotation = rotateIntoVecQ(state.prevDirection, direction2);
	fs[1].prevRotation *= state.prevRotation;
	fs[1].prevDirection = direction2;
	segments[1] = addStem(fork[1], fs[1], state, true);

	/** Create some pointers to make it easier to access the starting point
	of each stem. */
	DVertex *v1 = &this->vertices[fs[0].mesh][segments[0].vertexStart];
	DVertex *v2 = &this->vertices[fs[1].mesh][segments[1].vertexStart];
	DVertex *v0;
	{
		size_t start = state.segment.vertexStart;
		size_t count = state.segment.vertexCount;
		v0 = &this->vertices[state.mesh][start+count-sDivisions-1];
	}

	/** Use the fork directions to determine how the cross sections should
	be split in half. */
	int length = sDivisions / 2;
	int quad = sDivisions % 4 == 0;
	int offset;
	int edge1;
	int edge2;
	int midpoint = getForkMidpoint(2*sDivisions, direction1, direction2,
		state.prevDirection, state.prevRotation);

	if ((quad && midpoint % 2 == 0) || (!quad && midpoint % 2 != 0)) {
		midpoint /= 2;
		offset = 1;
		edge1 = midpoint - length / 2;
		edge2 = edge1 + length;
		edge1 += (edge1 < 0) * sDivisions;
		edge2 -= (edge2 >= sDivisions) * sDivisions;
	} else {
		midpoint /= 2;
		offset = 0;
		edge1 = midpoint - length / 2 + quad;
		edge2 = midpoint + length / 2 + 1;
		length -= 1;
		edge1 += (edge1 < 0) * sDivisions;
		edge2 -= (edge2 >= sDivisions) * sDivisions;

		size_t start = state.segment.vertexStart;
		size_t count = state.segment.vertexCount;
		vector<unsigned> &indices = this->indices[state.mesh];
		addExtraTriangles(indices, edge1, edge2, sDivisions,
			start + count - sDivisions - 1,
			segments[0].vertexStart, segments[1].vertexStart);
	}

	/** Intersect the cross sections on three planes. These points are used
	to create the tangents of the Bezier curves. */
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

	for (int i = 0; i <= length; i++) {
		int k = edge1 + i;
		k -= (k >= sDivisions) * sDivisions;
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
		v1[k].uv.y = v0[k].uv.y;
		insertCurve(sDivisions+1, cDivisions, size, false,
			&v0[k-size], &v1[k]);
	}
	for (int i = offset; i <= length - offset; i++) {
		int k = edge2 + i;
		k -= (k >= sDivisions) * sDivisions;
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
		v2[k].uv.y = v0[k].uv.y;
		insertCurve(sDivisions+1, cDivisions, size, false,
			&v0[k-size], &v2[k]);
	}
	if (offset > 0) {
		v2[edge2].position = v1[edge2].position;
		v2[edge1].position = v1[edge1].position;
		v2[edge2].normal = v1[edge2].normal;
		v2[edge1].normal = v1[edge1].normal;
		v2[edge2].tangent = v1[edge2].tangent;
		v2[edge1].tangent = v1[edge1].tangent;
	}
	for (int i = offset; i <= length - offset; i++) {
		int k1 = edge1 - 1 + offset - i;
		int k2 = edge1 + i;
		k1 += (k1 < 0) * sDivisions;
		k1 -= (k1 >= sDivisions) * sDivisions;
		k2 += (k2 < 0) * sDivisions;
		k2 -= (k2 >= sDivisions) * sDivisions;
		ray.origin = v1[k1].position;
		ray.direction = direction1;
		float t = intersectsPlane(ray, plane3);
		v1[k1].position = ray.origin + t*ray.direction;
		v2[k2].position = v1[k1].position;
		v1[k1].normal = normalize(v1[k1].normal + v2[k2].normal);
		v2[k2].normal = v1[k1].normal;
		v1[k1].tangent = normalize(v1[k1].tangent + v2[k2].tangent);
		v2[k2].tangent = v1[k1].tangent;
		insertCurve(sDivisions+1, cDivisions, size, true,
			&v1[k1], &v2[k2]);
	}
	if (offset == 1 && cDivisions > 1) {
		insertCurve(sDivisions+1, cDivisions, size,
			&v2[edge2], &v0[edge2]);
		insertCurve(sDivisions+1, cDivisions, size,
			&v2[edge1], &v0[edge1]);

	}
	for (int i = 0; i < cDivisions || i == 0; i++) {
		size_t a = i*(sDivisions+1);
		size_t b = a + sDivisions;
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
	setFork1UVs(state.segment.stem, sDivisions+1, cDivisions, &v0[-size]);
	setFork2UVs(fork[0], sDivisions+1, cDivisions, &v1[0]);
	setFork2UVs(fork[1], sDivisions+1, cDivisions, &v2[0]);

	if (cDivisions > 0)
		addForkTriangles(state, fs, fork, segments);

	for (int i = 0; i < 2; i++) {
		Stem *childFork[2];
		fork[i]->getFork(childFork);
		if (!isValidFork(fork[i], childFork))
			childFork[0] = childFork[1] = nullptr;
		addChildStems(fork[i], childFork, fs[i]);
	}

	return true;
}

void Mesh::addForkTriangles(const State &state, const State fs[2],
	Stem *fork[2], const Segment segments[2])
{
	const int cDivisions = fork[0]->getPath().getInitialDivisions();
	const int sDivisions = state.segment.stem->getSectionDivisions() + 1;
	const size_t vsize = getForkVertexCount(fork[0]);
	const size_t isize = getForkIndexCount(fork[0]);
	const size_t istart = state.segment.indexStart;
	const size_t icount = state.segment.indexCount;
	unsigned *indices = &this->indices[state.mesh][istart+icount-isize];
	const size_t vstart = state.segment.vertexStart;
	const size_t vcount = state.segment.vertexCount;
	size_t i1 = vstart + vcount - vsize - sDivisions*2;
	size_t i2 = i1 + sDivisions;

	for (int i = 0; i < cDivisions; i++) {
		indices += insertTriangleRing(i1, i2, sDivisions-1, indices);
		i1 = i2;
		i2 += sDivisions;
	}

	indices = &this->indices[fs[0].mesh][segments[0].indexStart];
	i1 = segments[0].vertexStart;
	i2 = i1 + sDivisions;
	for (int i = 0; i < cDivisions; i++) {
		indices += insertTriangleRing(i1, i2, sDivisions-1, indices);
		i1 = i2;
		i2 += sDivisions;
	}

	indices = &this->indices[fs[1].mesh][segments[1].indexStart];
	i1 = segments[1].vertexStart;
	i2 = i1 + sDivisions;
	for (int i = 0; i < cDivisions; i++) {
		indices += insertTriangleRing(i1, i2, sDivisions-1, indices);
		i1 = i2;
		i2 += sDivisions;
	}
}

/** Return the point index that is between the dividing line of the section. */
int Mesh::getForkMidpoint(int divisions, Vec3 direction1, Vec3 direction2,
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
	Vec3 a = normalize(direction1 + direction2);
	Quat sectionRotation = rotateIntoVecQ(a, direction);
	direction1 = rotate(sectionRotation, direction1);
	direction2 = rotate(sectionRotation, direction2);
	Vec3 middle = normalize(projectOntoPlane(direction1, direction));
	Vec3 division = cross(direction, middle);
	if (d1 > d2 || d1 > d3)
		division = -division;

	float x = dot(point, middle);
	if (x > 1.0f)
		x = 1.0f;
	if (x < -1.0f)
		x = -1.0f;
	float delta = 2.0f * pi / divisions;
	float theta = std::acos(x);
	if (dot(point, division) < 0.0f)
		theta = 2.0f*pi-theta;

	int midpoint;
	if (divisions % 4 == 0)
		midpoint = std::round(theta / delta);
	else
		midpoint = std::floor(theta / delta);
	midpoint -= (midpoint >= divisions) * divisions;
	return midpoint;
}

/** Create two cross sections and connect them with Bezier curves. */
void Mesh::createBranchCollar(State &state, Segment parentSegment)
{
	Stem *stem = state.segment.stem;
	State originalState = state;

	addSection(state, rotateSection(state), this->crossSection);
	size_t start = this->vertices[state.mesh].size();
	reserveBranchCollarSpace(stem, state.mesh);
	state.prevIndex = this->vertices[state.mesh].size();
	state.texOffset = 0.0f;
	state.section = stem->getPath().getInitialDivisions() + 1;
	state.prevIndex = this->vertices[state.mesh].size();
	addSection(state, rotateSection(state), this->crossSection);

	state.section = insertCollar(state.segment, parentSegment, start);
	if (state.section == 0)
		state = originalState;
}

/** Add a subsequent triangle ring to connect the collar with the stem. */
void Mesh::connectCollar(const State &state, bool fork)
{
	Stem *stem = state.segment.stem;
	size_t sections = state.segment.stem->getPath().getSize();
	bool a = !fork && state.section > 0 && state.section < sections;
	bool b = fork && state.section+1 < sections;
	if (a || b)
		addTriangleRing(state.prevIndex,
			this->vertices[state.mesh].size(),
			stem->getSectionDivisions(), state.mesh);
}

/** Return the amount of memory needed for the branch collar. */
inline size_t getBranchCollarSize(Stem *stem)
{
	int cd = stem->getPath().getInitialDivisions();
	return (stem->getSectionDivisions()+1) * cd;
}

/** Cross sections are usually created one at a time and then connected with
triangles. Branch collars are created by connecting cross sections with
splines, which means that many cross sections are created at a time. Reserving
memory in advance enables offsets to be used to maintain an identical vertex
layout. */
void Mesh::reserveBranchCollarSpace(Stem *stem, int mesh)
{
	size_t size = getBranchCollarSize(stem) + this->vertices[mesh].size();
	this->vertices[mesh].resize(size);
}

/** The first step in generating the branch collar is scaling the first cross
section of the stem along the parent stem. */
Mat4 Mesh::getBranchCollarScale(Stem *child, Stem *parent)
{
	Mat4 scale = identity();

	if (!parent) {
		scale[0][0] = child->getSwelling().x;
		scale[2][2] = child->getSwelling().y;
		return scale;
	}

	scale[2][2] = child->getSwelling().x;
	scale[1][1] = child->getSwelling().y;

	float position = child->getDistance();
	Vec3 yaxis = parent->getPath().getIntermediateDirection(position);
	Vec3 xaxis = child->getPath().getDirection(0);
	xaxis = normalize(cross(cross(yaxis, xaxis), yaxis));
	Vec3 zaxis = normalize(cross(yaxis, xaxis));

	Mat4 axes = identity();
	axes.vectors[0] = toVec4(xaxis, 0.0f);
	axes.vectors[1] = toVec4(yaxis, 0.0f);
	axes.vectors[2] = toVec4(zaxis, 0.0f);

	return axes * scale * transpose(axes);
}

Vec3 getSurfaceNormal(Vec3 p1, Vec3 p2, Vec3 p3, Vec3 n1, Vec3 n2, Vec3 n3,
	Vec3 intersection)
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

/** Project a point from a cross section on its parent's surface. */
DVertex Mesh::moveToSurface(DVertex vertex, Ray ray, Segment parent,
	size_t firstIndex)
{
	float length = magnitude(ray.direction);
	ray.direction = normalize(ray.direction);

	if (!parent.stem) {
		Plane plane;
		plane.normal = Vec3(0.0f, -1.0f, 0.0f);
		plane.point = Vec3(0.0f, 0.0f, 0.0f);
		float t = intersectsPlane(ray, plane);
		Vec3 offset = (t - length) * ray.direction;
		vertex.position += offset;
		return vertex;
	}

	unsigned mesh = parent.stem->getMaterial(Stem::Outer);
	DVertex *vertices = &this->vertices[mesh][0];
	unsigned *indices = &this->indices[mesh][0];

	float s;
	float t = std::numeric_limits<float>::max();
	size_t lastIndex = parent.indexStart + parent.indexCount;
	size_t offset = 0;

	while (offset < parent.indexCount) {
		size_t i = firstIndex + offset;
		size_t j = firstIndex - offset;
		offset += 3;

		if (i < lastIndex) {
			size_t i1 = indices[i];
			Vec3 p1 = vertices[i1].position;
			size_t i2 = indices[i+1];
			Vec3 p2 = vertices[i2].position;
			size_t i3 = indices[i+2];
			Vec3 p3 = vertices[i3].position;
			s = intersectsFrontTriangle(ray, p1, p2, p3);
			if (s != 0.0f) {
				t = s;
				vertex.normal = getSurfaceNormal(p1, p2, p3,
					vertices[i1].normal,
					vertices[i2].normal,
					vertices[i3].normal,
					t*ray.direction + ray.origin);
				break;
			}
		}
		if (j >= parent.indexStart && offset <= firstIndex) {
			size_t j1 = indices[j];
			Vec3 p1 = vertices[j1].position;
			size_t j2 = indices[j+1];
			Vec3 p2 = vertices[j2].position;
			size_t j3 = indices[j+2];
			Vec3 p3 = vertices[j3].position;
			s = intersectsFrontTriangle(ray, p1, p2, p3);
			if (s != 0.0f) {
				t = s;
				vertex.normal = getSurfaceNormal(p1, p2, p3,
					vertices[j1].normal,
					vertices[j2].normal,
					vertices[j3].normal,
					t*ray.direction + ray.origin);
				break;
			}
		}
	}

	if (t != std::numeric_limits<float>::max()) {
		Vec3 offset = (length - t) * ray.direction;
		vertex.normal = normalize(vertex.normal);
		vertex.position -= offset;
	} else {
		vertex.position.x = std::numeric_limits<float>::infinity();
		vertex.position.y = std::numeric_limits<float>::infinity();
		vertex.position.z = std::numeric_limits<float>::infinity();
	}

	return vertex;
}

/** Return a starting point for branch collar triangle intersections. */
inline size_t getTriangleOffset(const Segment &parent, const Segment &child)
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

inline void insertCurve(Vec3 c[4], int degree, DVertex v, int cDivisions,
	int sDivisions,  DVertex *buffer)
{
	float delta = 1.0f / (cDivisions + 1);
	float t = delta;
	if (degree == 3)
		for (int i = 0; i < cDivisions; i++, t += delta) {
			v.position = getCubicBezier(t, c[0], c[1], c[2], c[3]);
			buffer[sDivisions*i] = v;
		}
	else
		for (int i = 0; i < cDivisions; i++, t += delta) {
			v.position = getQuadraticBezier(t, c[0], c[1], c[3]);
			buffer[sDivisions*i] = v;
		}
}

size_t Mesh::insertCollar(Segment child, Segment parent, size_t vertexStart)
{
	const unsigned mesh = child.stem->getMaterial(Stem::Outer);
	const Path &path = child.stem->getPath();
	const int sDivisions = child.stem->getSectionDivisions() + 1;
	const int cDivisions = path.getInitialDivisions();
	size_t collarSize = getBranchCollarSize(child.stem);
	Mat4 scale = getBranchCollarScale(child.stem, parent.stem);
	size_t triangleOffset = getTriangleOffset(parent, child);

	Vec3 direction;
	int degree = path.getSpline().getDegree();
	if (degree == 3) {
		auto controls = path.getSpline().getControls();
		direction = controls[3] - controls[2];
	}

	for (int i = 0; i < sDivisions; i++) {
		size_t index = child.vertexStart + i;
		size_t index2 = index + collarSize + sDivisions;

		Ray ray;
		DVertex p1 = this->vertices[mesh][index];
		DVertex p2;
		p2.normal = p1.normal;
		p2.tangent = p1.tangent;
		p2.tangentScale = p1.tangentScale;
		p2.position = p1.position - child.stem->getLocation();
		p2.position = scale.apply(p2.position, 1.0f);
		p2.position += child.stem->getLocation();
		ray.origin = this->vertices[mesh][index2].position;
		ray.direction = p2.position - ray.origin;
		p2 = moveToSurface(p2, ray, parent, triangleOffset);
		if (std::isinf(p2.position.x)) {
			this->vertices[mesh].resize(child.vertexStart);
			this->indices[mesh].resize(child.indexStart);
			return 0;
		}
		p2.weights = this->vertices[mesh][index].weights;
		p2.indices = this->vertices[mesh][index].indices;
		this->vertices[mesh][index] = p2;

		ray.direction = p1.position - ray.origin;
		p1 = moveToSurface(p1, ray, parent, triangleOffset);
		if (std::isinf(p1.position.x)) {
			this->vertices[mesh].resize(child.vertexStart);
			this->indices[mesh].resize(child.indexStart);
			return 0;
		}

		Vec3 curve[4];
		curve[0] = p2.position;
		curve[1] = p1.position;
		curve[2] = this->vertices[mesh][index2].position - direction;
		curve[3] = this->vertices[mesh][index2].position;
		insertCurve(curve, degree, p2, cDivisions, sDivisions,
			&this->vertices[mesh][vertexStart+i]);
	}

	size_t index1 = child.vertexStart;
	size_t index2 = child.vertexStart + sDivisions;
	for (int i = 0; i <= cDivisions; i++) {
		addTriangleRing(index1, index2, sDivisions-1, mesh);
		index1 = index2;
		index2 += sDivisions;
	}

	index1 = child.vertexStart;
	index2 = vertexStart + collarSize;
	setBranchCollarNormals(index1, index2, mesh, sDivisions, cDivisions);
	setBranchCollarUVs(index2, child.stem, mesh, sDivisions, cDivisions);
	return cDivisions + 2;
}

/** Interpolate normals from the first cross section of the stem with normals
from the first cross section after the branch collar. */
void Mesh::setBranchCollarNormals(size_t index1, size_t index2, int mesh,
	int sDivisions, int cDivisions)
{
	DVertex *buffer1 = &this->vertices[mesh][0];
	DVertex *buffer2 = &this->vertices[mesh][0];
	for (int i = 0; i < sDivisions; i++) {
		Vec3 normal1 = buffer1[index1].normal;
		Vec3 normal2 = buffer2[index2].normal;
		Vec3 tangent1 = buffer1[index1].tangent;
		Vec3 tangent2 = buffer2[index2].tangent;

		for (int j = 1; j <= cDivisions; j++) {
			float t = j/static_cast<float>(cDivisions);
			float a = 2.0f*t*t;
			float b = -2.0f*t*t + 4.0f*t - 1.0f;
			t = a * (t <= 0.5f) + b * (t > 0.5f);

			Vec3 normal = lerp(normal1, normal2, t);
			normal = normalize(normal);
			Vec3 tangent = lerp(tangent1, tangent2, t);
			tangent = normalize(tangent);

			size_t offset = j * sDivisions;
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
void Mesh::setBranchCollarUVs(size_t lastIndex, Stem *stem, int mesh,
	int sDivisions, int cDivisions)
{
	DVertex *buffer = &this->vertices[mesh][lastIndex];
	float radius = this->plant->getRadius(stem, 1);
	float aspect = getAspect(this->plant, stem);

	for (int i = 0; i < sDivisions; i++) {
		Vec2 uv = buffer[i].uv;
		size_t index = i;

		for (int j = cDivisions; j >= 0; j--) {
			Vec3 p1 = buffer[index].position;
			index -= sDivisions;
			Vec3 p2 = buffer[index].position;
			float length = magnitude(p2 - p1);
			uv.y -= (length * aspect) / (radius * 2.0f * pi);
			buffer[index].uv = uv;
		}
	}
}

void Mesh::capStem(Stem *stem, int stemMesh, size_t section)
{
	long mesh = stem->getMaterial(Stem::Inner);
	size_t index = section;
	size_t divisions = stem->getSectionDivisions();
	float rotation = 2.0f * pi / divisions;
	float angle = 0.0f;
	section = this->vertices[mesh].size();

	for (size_t i = 0; i <= divisions; i++, index++, angle += rotation) {
		DVertex vertex = this->vertices[stemMesh][index];
		vertex.uv.x = std::cos(angle) * 0.5f + 0.5f;
		vertex.uv.y = std::sin(angle) * 0.5f + 0.5f;
		this->vertices[mesh].push_back(vertex);
	}

	for (index = 0; index < divisions/2 - 1; index++) {
		addTriangle(mesh,
			section + index,
			section + divisions - index - 1,
			section + index + 1);
		addTriangle(mesh,
			section + index + 1,
			section + divisions - index - 1,
			section + divisions - index - 2);
	}

	if ((divisions & 1) != 0) { /* is odd */
		size_t lastSection = section + index;
		addTriangle(mesh, lastSection, lastSection+2, lastSection+1);
	}
}

void Mesh::addLeaves(Stem *stem, const State &state)
{
	auto leaves = stem->getLeaves();
	unsigned index = 0;
	for (auto it = leaves.begin(); it != leaves.end(); it++)
		addLeaf(stem, index++, state);
}

void Mesh::addLeaf(Stem *stem, unsigned leafIndex, const State &state)
{
	Leaf *leaf = stem->getLeaf(leafIndex);
	long mesh = leaf->getMaterial();
	Segment leafSegment;
	leafSegment.leafIndex = leafIndex;
	leafSegment.stem = stem;
	leafSegment.vertexStart = this->vertices[mesh].size();
	leafSegment.indexStart = this->indices[mesh].size();

	Vec2 weights;
	Vec2 indices;
	if (stem->hasJoints()) {
		float position = leaf->getPosition();
		auto pair = getJoint(position, stem);
		size_t index = pair.second.getPathIndex();
		float jointPosition = stem->getPath().getDistance(index);
		float offset = position - jointPosition;
		setJointInfo(stem, offset, pair.first, weights, indices);
	} else {
		weights.x = 1.0f;
		weights.y = 0.0f;
		indices.x = static_cast<float>(state.jointID);
		indices.y = indices.x;
	}

	Geometry geom = transformLeaf(leaf, stem);
	size_t vsize = this->vertices[mesh].size();
	for (DVertex vertex : geom.getPoints()) {
		vertex.indices = indices;
		vertex.weights = weights;
		this->vertices[mesh].push_back(vertex);
	}
	for (unsigned i : geom.getIndices())
		this->indices[mesh].push_back(i + vsize);

	leafSegment.vertexCount = this->vertices[mesh].size();
	leafSegment.vertexCount -= leafSegment.vertexStart;
	leafSegment.indexCount = this->indices[mesh].size();
	leafSegment.indexCount -= leafSegment.indexStart;
	this->leafSegments[mesh].emplace(LeafID(stem, leafIndex), leafSegment);
}

Geometry Mesh::transformLeaf(const Leaf *leaf, const Stem *stem)
{
	const Path path = stem->getPath();
	Vec3 location = stem->getLocation();
	float position = leaf->getPosition();
	Vec3 direction;

	if (position >= 0.0f && position < path.getLength()) {
		direction = path.getIntermediateDirection(position);
		location += path.getIntermediate(position);
	} else {
		direction = path.getDirection(path.getSize() - 1);
		location += path.get().back();
	}

	Geometry geom = this->plant->getLeafMesh(leaf->getMesh());
	geom.transform(leaf->getRotation(), leaf->getScale(), location);
	return geom;
}

/** Stem descendants might not have joints and the parent state is needed to
determine what joint ancestors are influenced by. */
void Mesh::setInitialJointState(State &state, const State &parentState)
{
	const Stem *stem = state.segment.stem;
	const Stem *parent = stem->getParent();
	state.jointID = 0;
	state.jointIndex = 0;
	state.jointOffset = 0.0f;
	const vector<Joint> joints = stem->getJoints();

	if (joints.empty() && (!parent || !parent->hasJoints())) {
		state.jointID = parentState.jointID;
	} else if (joints.empty()) {
		float position = stem->getDistance();
		auto pair = getJoint(position, stem->getParent());
		state.jointID = pair.second.getID();
		state.jointIndex = pair.first;
	} else {
		Joint joint = joints.front();
		state.jointID = joint.getID();
	}
}

pair<size_t, Joint> Mesh::getJoint(float position, const Stem *stem)
{
	size_t index = stem->getPath().getIndex(position);
	const vector<Joint> joints = stem->getJoints();
	size_t jointIndex = 0;
	for (auto it = joints.begin(); it != joints.end(); it++) {
		size_t pathIndex = it->getPathIndex();
		if (pathIndex > index) {
			if (it != joints.begin()) {
				jointIndex--;
				it--;
			}
			return pair<size_t, Joint>(jointIndex, *it);
		}
		jointIndex++;
	}
	return pair<size_t, Joint>(jointIndex-1, joints.back());
}

void Mesh::incrementJoint(State &state, const vector<Joint> &joints)
{
	if (state.jointIndex + 1 < joints.size()) {
		Joint nextJoint = joints[state.jointIndex + 1];
		if (nextJoint.getPathIndex() == state.section) {
			state.jointIndex++;
			state.jointID = nextJoint.getID();
			state.jointOffset = 0.0f;
		}
	}
}

/** Update the current joint and set the joint indices and weights. */
void Mesh::updateJointState(State &state, Vec2 &indices, Vec2 &weights)
{
	const Stem *stem = state.segment.stem;
	const Path &path = stem->getPath();
	const vector<Joint> joints = stem->getJoints();
	incrementJoint(state, joints);
	size_t pathIndex = joints[state.jointIndex].getPathIndex();

	if (state.jointIndex == 0 && state.section <= pathIndex) {
		weights.x = 1.0f;
		weights.y = 0.0f;
		indices.x = static_cast<float>(state.jointID);
		indices.y = indices.x;
	} else if (state.section == 0 || state.section == path.getSize() - 1) {
		weights.x = 1.0f;
		weights.y = 0.0f;
		indices.x = static_cast<float>(state.jointID);
		indices.y = indices.x;
	} else if (state.section == pathIndex) {
		unsigned prevID = joints[state.jointIndex-1].getID();
		weights.x = 0.5f;
		weights.y = 0.5f;
		indices.x = static_cast<float>(state.jointID);
		indices.y = static_cast<float>(prevID);
	} else {
		if (state.section != pathIndex) {
			Vec3 point1 = path.get(state.section);
			Vec3 point2 = path.get(state.section - 1);
			state.jointOffset += magnitude(point1 - point2);
		}
		setJointInfo(stem, state.jointOffset, state.jointIndex,
			weights, indices);
	}
}

void Mesh::setJointInfo(const Stem *stem, float jointOffset, size_t jointIndex,
	Vec2 &weights, Vec2 &indices)
{
	const vector<Joint> joints = stem->getJoints();
	const Path &path = stem->getPath();
	size_t pathIndex = joints[jointIndex].getPathIndex();
	unsigned jointID = joints[jointIndex].getID();

	float ratio;
	float distance;
	bool lastJoint = jointIndex + 1 >= joints.size();
	if (lastJoint) {
		size_t start = pathIndex;
		size_t end = path.getSize() - 1;
		distance = path.getDistance(start, end);
		ratio = jointOffset / distance;
	} else {
		Joint nextJoint = joints[jointIndex + 1];
		size_t start = pathIndex;
		size_t end = nextJoint.getPathIndex();
		distance = path.getDistance(start, end);
		ratio = jointOffset / distance;
	}

	bool first = ratio < 0.5f && jointIndex == 0;
	bool last = ratio > 0.5f && lastJoint;
	if (ratio == 0.5f || first || last) {
		weights.x = 1.0f;
		weights.y = 0.0f;
		indices.x = static_cast<float>(jointID);
		indices.y = indices.x;
	} else if (ratio > 0.5f) {
		int nextID = joints[jointIndex + 1].getID();
		indices.x = static_cast<float>(jointID);
		indices.y = static_cast<float>(nextID);
		float ratio = jointOffset / distance - 0.5f;
		weights.x = 1.0f - ratio;
		weights.y = ratio;
	} else {
		int prevID = joints[jointIndex - 1].getID();
		indices.x = static_cast<float>(jointID);
		indices.y = static_cast<float>(prevID);
		float ratio = jointOffset / distance;
		weights.x = 0.5f + ratio;
		weights.y = 0.5f - ratio;
	}
}

size_t Mesh::insertTriangleRing(size_t i1, size_t i2, int divisions,
	unsigned *indices)
{
	size_t index = 0;
	for (int i = 0; i < divisions; i++) {
		indices[index++] = i2;
		indices[index++] = i2 + 1;
		indices[index++] = i1;
		i2++;
		indices[index++] = i1;
		indices[index++] = i2;
		indices[index++] = i1 + 1;
		i1++;
	}
	return index;
}

/** Compute indices between the cross section just generated and the cross
section that still needs to be generated. */
void Mesh::addTriangleRing(size_t i1, size_t i2, int divisions, int mesh)
{
	for (int i = 0; i < divisions; i++) {
		addTriangle(mesh, i2, i2 + 1, i1);
		i2++;
		addTriangle(mesh, i1, i2, i1 + 1);
		i1++;
	}
}

void Mesh::addTriangle(int mesh, int a, int b, int c)
{
	this->indices[mesh].push_back(a);
	this->indices[mesh].push_back(b);
	this->indices[mesh].push_back(c);
}

void Mesh::initBuffer()
{
	size_t size = this->plant->getMaterials().size();
	this->vertices.resize(size);
	this->indices.resize(size);
	this->stemSegments.resize(size);
	this->leafSegments.resize(size);
	for (size_t i = 0; i < size; i++) {
		this->vertices[i].clear();
		this->indices[i].clear();
		this->stemSegments[i].clear();
		this->leafSegments[i].clear();
	}
}

/** Geometry is divided into different groups depending on material.
Geometry is later stored in the same vertex buffer but is separated based
on material to minimize draw calls. This method updates the indices to what
they should be in the final merged vertex buffer. */
void Mesh::updateSegments()
{
	if (!this->indices.empty()) {
		unsigned vsize = this->vertices[0].size();
		unsigned isize = this->indices[0].size();
		for (unsigned mesh = 1; mesh < this->indices.size(); mesh++) {
			for (unsigned &index : this->indices[mesh])
				index += vsize;
			for (auto &pair : this->stemSegments[mesh]) {
				Segment *segment = &pair.second;
				segment->vertexStart += vsize;
				segment->indexStart += isize;
			}
			for (auto &pair : this->leafSegments[mesh]) {
				Segment *segment = &pair.second;
				segment->vertexStart += vsize;
				segment->indexStart += isize;
			}
			vsize += this->vertices[mesh].size();
			isize += this->indices[mesh].size();
		}
	}
}

size_t Mesh::getMeshCount() const
{
	return this->indices.size();
}

size_t Mesh::getVertexCount() const
{
	size_t size = 0;
	for (auto &mesh : this->vertices)
		size += mesh.size();
	return size;
}

size_t Mesh::getIndexCount() const
{
	size_t size = 0;
	for (auto &mesh : this->indices)
		size += mesh.size();
	return size;
}

unsigned Mesh::getMaterialIndex(int mesh) const
{
	return mesh;
}

vector<DVertex> Mesh::getVertices() const
{
	vector<DVertex> object;
	for (auto it = this->vertices.begin(); it != this->vertices.end(); it++)
		object.insert(object.end(), it->begin(), it->end());
	return object;
}

vector<unsigned> Mesh::getIndices() const
{
	vector<unsigned> object;
	for (auto it = this->indices.begin(); it != this->indices.end(); it++)
		object.insert(object.end(), it->begin(), it->end());
	return object;
}

const vector<DVertex> *Mesh::getVertices(int mesh) const
{
	return &this->vertices.at(mesh);
}

const vector<unsigned> *Mesh::getIndices(int mesh) const
{
	return &this->indices.at(mesh);
}

map<Mesh::LeafID, Segment> Mesh::getLeaves(int mesh) const
{
	return this->leafSegments.at(mesh);
}

size_t Mesh::getLeafCount(int mesh) const
{
	return this->leafSegments.at(mesh).size();
}

Segment Mesh::findStem(Stem *stem) const
{
	for (size_t i = 0; i < this->stemSegments.size(); i++) {
		auto it = this->stemSegments[i].find(stem);
		if (it != this->stemSegments[i].end())
			return it->second;
	}
	return Segment();
}

Segment Mesh::findLeaf(LeafID leaf) const
{
	for (size_t i = 0; i < this->leafSegments.size(); i++) {
		auto it = this->leafSegments[i].find(leaf);
		if (it != this->leafSegments[i].end())
			return it->second;
	}
	return Segment();
}

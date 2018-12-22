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

using pg::Vec3;
using pg::Mat4;

pg::Mesh::Mesh(pg::Plant *plant)
{
	this->plant = plant;
	vertices.reserve(1000);
	indices.reserve(1000);
}

bool pg::Mesh::generate()
{
	Stem *stem = plant->getRoot();
	segments.clear();
	vertices.clear();
	indices.clear();
	size_t vertexCount = 0;
	size_t indexCount = 0;
	addStem(stem);
	bool resizedVertexBuffer = vertices.size() != vertexCount;
	bool resizedIndexBuffer = indices.size() != indexCount;
	return resizedIndexBuffer || resizedVertexBuffer;
}

const std::vector<float> *pg::Mesh::getVertices() const
{
	return &vertices;
}

const std::vector<unsigned> *pg::Mesh::getIndices() const
{
	return &indices;
}

const std::vector<pg::Segment> *pg::Mesh::getSegments() const
{
	return &segments;
}

pg::Segment pg::Mesh::find(pg::Stem *stem) const
{
	Segment segment = {0, 0, 0, 0, 0};
	for (size_t i = 0; i < segments.size(); i++) {
		if (segments[i].stem == stem) {
			segment = segments[i];
			break;
		}
	}
	return segment;
}

void pg::Mesh::addSectionPoints(Mat4 t, float radius, int count)
{
	const float rotation = 2.0f*M_PI/count;
	float angle = 0.0f;

	for (int i = 0; i < count; i++) {
		Vec3 point = {std::cos(angle), 0.0f, std::sin(angle)};
		Vec3 normal = pg::normalize(point);
		point = radius*point;

		normal = pg::normalize(pg::toVec3(t*pg::toVec4(normal, 0.0f)));
		point = pg::toVec3(t*pg::toVec4(point, 1.0f));
		addPoint(point, normal);

		angle += rotation;
	}
}

Mat4 pg::Mesh::getSectionTransform(Stem *stem, size_t section, float offset)
{
	Vec3 up = {0.0f, 1.0f, 0.0f};
	Vec3 point = stem->getPath().get(section);
	Vec3 direction = stem->getPath().getAverageDirection(section);
	Vec3 location = stem->getLocation() + point + offset * direction;
	Mat4 rotation = pg::rotateIntoVec(up, direction);
	Mat4 translation = pg::translate(location);
	return translation * rotation;
}

void pg::Mesh::addSection(Stem *stem, size_t section, float offset)
{
	Mat4 transform = getSectionTransform(stem, section, offset);
	float radius = stem->getPath().getRadius(section);
	addSectionPoints(transform, radius, stem->getResolution());
}

void pg::Mesh::addRectangle(size_t *s1, size_t *s2)
{
	indices.push_back(*s2);
	indices.push_back(++(*s2));
	indices.push_back(*s1);

	indices.push_back(*s1);
	indices.push_back(*s2);
	indices.push_back(++(*s1));
}

void pg::Mesh::addLastRectangle(size_t a, size_t b, size_t iA, size_t iB)
{
	addTriangle(iB, iA, a);
	addTriangle(a, b, iB);
}

void pg::Mesh::addTriangleRing(size_t csA, size_t csB, int divisions)
{
	size_t initCsA = csA;
	size_t initCsB = csB;

	for (int i = 0; i < divisions - 1; i++)
		addRectangle(&csA, &csB);
	addLastRectangle(csA, csB, initCsA, initCsB);
}

void pg::Mesh::addRectangles(size_t csA, size_t csB, int start, int end,
	int divisions)
{
	size_t initCsA = csA;
	size_t initCsB = csB;
	int i;

	for (i = 0; i < divisions-1; i++)
		if (i >= start && i <= end)
			addRectangle(&csA, &csB);
		else {
			csA++;
			csB++;
		}

	if (i >= start && i <= end) {
		csA = initCsA + divisions - 1;
		csB = initCsB + divisions - 1;
		addLastRectangle(csA, csB, initCsA, initCsB);
	}
}

void pg::Mesh::capStem(Stem *stem, size_t section)
{
	size_t vertex = section / vertexSize;
	int divisions = stem->getResolution();
	int index;

	for (index = 0; index < divisions/2 - 1; index++) {
		indices.push_back(vertex + index);
		indices.push_back(vertex + divisions - index - 1);
		indices.push_back(vertex + index + 1);

		indices.push_back(vertex + index + 1);
		indices.push_back(vertex + divisions - index - 1);
		indices.push_back(vertex + divisions - index - 2);
	}

	if ((divisions & 1) != 0 /* is odd */) {
		indices.push_back(vertex + index);
		indices.push_back(vertex + index + 2);
		indices.push_back(vertex + index + 1);
	}
}

void pg::Mesh::addStem(Stem *stem, float offset)
{
	pg::Segment bufferSegment;

	if (std::isnan(stem->getLocation().x))
		return;

	bufferSegment.stem = stem;
	bufferSegment.vertexStart = vertices.size();
	bufferSegment.indexStart = indices.size();

	{ /* first cross section */
		size_t origIndex = vertices.size() / vertexSize;
		addSection(stem, 0, offset);
		size_t currentIndex = vertices.size() / vertexSize;
		addTriangleRing(origIndex, currentIndex, stem->getResolution());
	}

	for (int i = 1; i < stem->getPath().getSize() - 1; i++) {
		size_t origIndex = vertices.size() / vertexSize;
		addSection(stem, i);
		size_t currentIndex = vertices.size() / vertexSize;
		addTriangleRing(origIndex, currentIndex, stem->getResolution());
	}

	{ /* last cross section */
		size_t lastSection = vertices.size();
		addSection(stem, stem->getPath().getSize() - 1);
		capStem(stem, lastSection);
	}

	bufferSegment.vertexCount = vertices.size() - bufferSegment.vertexStart;
	bufferSegment.indexCount = indices.size() - bufferSegment.indexStart;
	segments.push_back(bufferSegment);

	{
		Stem *child = stem->getChild();
		while (child != nullptr) {
			addStem(child, 0.0f);
			child = child->getSibling();
		}
	}

	addLeaves(stem);
}

void pg::Mesh::addLeaves(Stem *stem)
{
	for (int i = 0; i < stem->getLeafCount(); i++) {
		Leaf *leaf = stem->getLeaf(i);
		VolumetricPath path = stem->getPath();
		Vec3 normal = {0.0f, 1.0f, 0.0f};
		Vec3 p[4];

		{
			pg::Vec2 s = leaf->getScale();
			p[0] = {0.5f * s.x, 0.0f, 0.0f};
			p[1] = {0.5f * s.x, 0.0f, 1.0f * s.y};
			p[2] = {-0.5f * s.x, 0.0f, 1.0f * s.y};
			p[3] = {-0.5f * s.x, 0.0f, 0.0f};
		}

		{
			Mat4 m = rotateXY(leaf->getTilt(), 0.0f);
			p[1] = pg::toVec3(m * pg::toVec4(p[1], 1.0f));
			p[2] = pg::toVec3(m * pg::toVec4(p[2], 1.0f));
		}

		Vec3 location = stem->getLocation();
		float position = leaf->getPosition();
		if (position >= 0.0f && position < path.getLength()) {
			Vec3 dir = path.getIntermediateDirection(position);
			rotateSideLeaf(p, normal, dir);
			location += path.getIntermediate(position);
		} else {
			Vec3 dir = path.getDirection(path.getSize() - 1);
			rotateEndLeaf(p, normal, dir);
			location += path.get().back();
		}

		for (int i = 0; i < 4; i++)
			p[i] += location;

		size_t index = vertices.size() / vertexSize;
		addPoint(p[0], normal);
		addPoint(p[1], normal);
		addPoint(p[2], normal);
		addPoint(p[3], normal);
		addTriangle(index, index + 1, index + 3);
		addTriangle(index + 1, index + 2, index + 3);
	}
}

void pg::Mesh::rotateSideLeaf(Vec3 (&p)[4], Vec3 &normal, Vec3 direction)
{
	Vec3 leafDirection = pg::normalize(pg::cross(direction, normal));
	Vec3 d = {0.0f, 0.0f, 1.0f};
	Mat4 m = pg::rotateIntoVec(d, leafDirection);

	for (int i = 0; i < 4; i++)
		p[i] = pg::toVec3(m * pg::toVec4(p[i], 1.0f));

	{
		Vec3 up = {0.0f, -1.0f, 0.0f};
		Vec3 d = pg::cross(up, direction);
		d = pg::cross(d, direction);
		d = pg::normalize(d);
		Mat4 m = pg::rotateIntoVec(normal, d);
		for (int i = 0; i < 4; i++)
			p[i] = pg::toVec3(m * pg::toVec4(p[i], 1.0f));
		normal = d;
	}

}

void pg::Mesh::rotateEndLeaf(Vec3 (&p)[4], Vec3 &normal, Vec3 direction)
{
	Vec3 d = {0.0f, 0.0f, 1.0f};
	Mat4 m = pg::rotateIntoVec(d, direction);

	normal = pg::toVec3(m * pg::toVec4(normal, 1.0f));
	for (int i = 0; i < 4; i++)
		p[i] = pg::toVec3(m * pg::toVec4(p[i], 1.0f));

	{
		Vec3 up = {0.0f, 1.0f, 0.0f};
		Vec3 leafDirection = pg::cross(direction, up);
		leafDirection = pg::cross(leafDirection, direction);
		leafDirection = pg::normalize(leafDirection);
		Mat4 m = pg::rotateIntoVec(normal, leafDirection);
		for (int i = 0; i < 4; i++)
			p[i] = pg::toVec3(m * pg::toVec4(p[i], 1.0f));
		normal = leafDirection;
	}
}

void pg::Mesh::addPoint(Vec3 point, Vec3 normal)
{
	vertices.push_back(point.x);
	vertices.push_back(point.y);
	vertices.push_back(point.z);
	vertices.push_back(normal.x);
	vertices.push_back(normal.y);
	vertices.push_back(normal.z);
}

void pg::Mesh::addTriangle(int a, int b, int c)
{
	indices.push_back(a);
	indices.push_back(b);
	indices.push_back(c);
}

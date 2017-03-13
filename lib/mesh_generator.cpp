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

#include <cmath>
#include "mesh_generator.h"

MeshGenerator::MeshGenerator()
{
	vertices.resize(400);
	indices.resize(400);
}

void MeshGenerator::resizeVertexBuffer(size_t additions)
{
	if (vbIndex + additions > vertices.size()) {
		vertices.resize(vertices.size() * 2);
		resized = true;
	}
}

void MeshGenerator::resizeIndexBuffer(size_t additions)
{
	if (ibIndex + additions > indices.size()) {
		indices.resize(indices.size() * 2);
		resized = true;
	}
}

void MeshGenerator::addCrossSectionPoints(Mat4 tran, float radius, int div)
{
	const size_t end = vbIndex + div * 6;
	const float rotation = 2.0f * M_PI / div;
	float angle = 0.0f;

	resizeVertexBuffer(div*6);

	while (vbIndex < end) {
		Vec3 point = {std::cos(angle), 0.0f, std::sin(angle)};
		Vec3 normal = normalize(point);
		point = radius * point;
		
		normal = normalize(toVec3(tran * toVec4(normal, 0.0f)));
		point = toVec3(tran * toVec4(point, 1.0f));
		
		vertices[vbIndex++] = point.x;
		vertices[vbIndex++] = point.y;
		vertices[vbIndex++] = point.z;
		vertices[vbIndex++] = normal.x;
		vertices[vbIndex++] = normal.y;
		vertices[vbIndex++] = normal.z;

		angle += rotation;
	}
}

float MeshGenerator::getRadius(Stem *stem, size_t index)
{
	float divisions = static_cast<float>(stem->getPath().getDivisions()-1);
	float t = static_cast<float>(index) / divisions;
	auto &c = stem->radiusCurve;
	float radius = getBezierPath(t, &c[0], c.size()).z * stem->radius;
	return radius < stem->minRadius ? stem->minRadius : radius;
}

Mat4 MeshGenerator::getCrossSectionTransform(Stem *stem, size_t i, float offset)
{
	Vec3 up = {0.0f, 1.0f, 0.0f};
	Vec3 point = stem->getPath().getGeneratedPoint(i);
	Vec3 direction = stem->getPath().getDirection(i);
	Vec3 location = stem->getLocation() + point + offset * direction;
	Mat4 rotation = rotateIntoVec(up, direction);
	Mat4 translation = translate(location);
	return translation * rotation;
}

void MeshGenerator::addCrossSection(Stem *stem, size_t i, float offset)
{
	Mat4 transform = getCrossSectionTransform(stem, i, offset);
	float radius = getRadius(stem, i);
	addCrossSectionPoints(transform, radius, stem->getResolution());
}

void MeshGenerator::addRectangle(size_t *csA, size_t *csB)
{
	resizeIndexBuffer(6);

	indices[ibIndex++] = *csB;
	indices[ibIndex++] = ++(*csB);
	indices[ibIndex++] = *csA;

	indices[ibIndex++] = *csA;
	indices[ibIndex++] = *csB;
	indices[ibIndex++] = ++(*csA);
}

void MeshGenerator::addLastRectangle(size_t a, size_t b, size_t iA, size_t iB)
{
	resizeIndexBuffer(6);

	indices[ibIndex++] = iB;
	indices[ibIndex++] = iA;
	indices[ibIndex++] = a;

	indices[ibIndex++] = a;
	indices[ibIndex++] = b;
	indices[ibIndex++] = iB;
}

void MeshGenerator::addTriangleRing(size_t csA, size_t csB, int divisions)
{
	size_t initCsA = csA;
	size_t initCsB = csB;

	for (int i = 0; i < divisions - 1; i++)
		addRectangle(&csA, &csB);
	addLastRectangle(csA, csB, initCsA, initCsB);
}

void MeshGenerator::addRectangles(size_t csA, size_t csB, int start, int end,
		int divisions)
{
	size_t initCsA = csA;
	size_t initCsB = csB;
	int i = 0;
	
	for (; i < divisions-1; i++)
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

void MeshGenerator::capStem(Stem *stem, size_t crossSection)
{
	size_t vertex = crossSection / 6;
	int divisions = stem->getResolution();
	int index;

	for (index = 0; index < divisions/2 - 1; index++) {
		resizeIndexBuffer(6);

		indices[ibIndex++] = vertex + index;
		indices[ibIndex++] = vertex + divisions - index - 1;
		indices[ibIndex++] = vertex + index + 1;

		indices[ibIndex++] = vertex + index + 1;
		indices[ibIndex++] = vertex + divisions - index - 1;
		indices[ibIndex++] = vertex + divisions - index - 2;
	}

	if ((divisions & 1) != 0 /* is odd */) {
		resizeIndexBuffer(3);
		indices[ibIndex++] = vertex + index;
		indices[ibIndex++] = vertex + index + 2;
		indices[ibIndex++] = vertex + index + 1;
	}
}

Vec3 MeshGenerator::getCrossSectionNormal(size_t index)
{
	Vec3 a = {vertices[index+3], vertices[index+4], vertices[index+5]};
	Vec3 b = {vertices[index+9], vertices[index+10], vertices[index+11]};
	return normalize(cross(b, a));
}

float MeshGenerator::getStemAngle(size_t index, Vec3 direction)
{
	Vec3 normal = getCrossSectionNormal(index);
	Vec3 up = {0.0f, 1.0f, 0.0f};
	Mat4 rotation = rotateIntoVec(normal, up);
	
	direction = toVec3(rotation * toVec4(direction, 0.0f));
	
	float angle = std::atan(direction.z / direction.x);
	if (direction.z < 0.0f && direction.x >= 0.0f)
		angle += 2.0f * M_PI;
	else if (direction.z < 0.0f && direction.x < 0.0f)
		angle += M_PI;
	else if (direction.z >= 0.0f && direction.x < 0.0f)
		angle += M_PI;
	return angle;
}

void MeshGenerator::getBounds(Vec3 direction, int divisions, size_t index,
		int *lowerBound, int *upperBound, float *angle)
{
	float minAngle = 2.0f*M_PI/divisions;

	*angle = getStemAngle(index, direction);
	*lowerBound = *upperBound = std::round(*angle/minAngle);

	for (int i = 1; i < divisions/2; i++) {
		float a = (*lowerBound - 1.0f)*minAngle;
		float b = (*upperBound + 1.0f)*minAngle;
		a = std::abs(*angle - a - minAngle/2.0f);
		b = std::abs(*angle - b - minAngle/2.0f);
		if (a < b)
			(*lowerBound)--;
		else
			(*upperBound)++;
	}

	if (*lowerBound < 0)
		*lowerBound += divisions;
	if (*upperBound >= divisions)
		*upperBound -= divisions;
}

bool MeshGenerator::isDichotomousTwisted(float angle, int divisions)
{
	int m = static_cast<int>(angle*180.0f/M_PI) - 90;
	int l = 360.0f / divisions;
	return m%l >= l/2;
}

void MeshGenerator::connectDichotomous(size_t a, size_t b, size_t c,
		int divisions, Vec3 direction)
{
	int upperBound;
	int lowerBound;
	float angle;
	bool twisted;
	int length;
	int x;
	int y;

	getBounds(direction, divisions, a, &lowerBound, &upperBound, &angle);

	a /= 6;
	b /= 6;
	c /= 6;

	if (lowerBound > upperBound)
		addRectangles(a, b, upperBound+1, lowerBound-1, divisions);
	else {
		addRectangles(a, b, 0, lowerBound-1, divisions);
		addRectangles(a, b, upperBound+1, divisions, divisions);
	}

	if (lowerBound < upperBound)
		addRectangles(a, c, lowerBound, upperBound, divisions);
	else {
		addRectangles(a, c, 0, upperBound, divisions);
		addRectangles(a, c, lowerBound, divisions, divisions);
	}
	
	twisted = isDichotomousTwisted(angle, divisions);

	x = upperBound+1 >= divisions ? upperBound+1-divisions : upperBound+1;
	resizeIndexBuffer(3);
	indices[ibIndex++] = b + x;
	indices[ibIndex++] = a + x;
	indices[ibIndex++] = c + x;
	y = x;
	if (twisted && divisions%2 != 0)
		y = y+1 < divisions ? y+1 : 0;

	length = divisions/2 + (twisted || divisions%2 == 0 ? 0 : 1);
	for (int i = 0; i < length; i++) {
		resizeIndexBuffer(3);
		if (twisted) {
			indices[ibIndex++] = b + (x-1 < 0 ? divisions-1 : x-1);
			indices[ibIndex++] = b + x;
			indices[ibIndex++] = c + y;
			y = y+1 < divisions ? y+1 : 0;
			x = x-1 < 0 ? divisions-1 : x-1;
		} else {
			indices[ibIndex++] = c + x;
			indices[ibIndex++] = c + (x+1 < divisions ? x+1 : 0);
			indices[ibIndex++] = b + y;
			y = y-1 < 0 ? divisions-1 : y-1;
			x = x+1 < divisions ? x+1 : 0;
		}
	}

	resizeIndexBuffer(3);
	indices[ibIndex++] = a + lowerBound;
	indices[ibIndex++] = b + lowerBound;
	indices[ibIndex++] = c + lowerBound;
	x = y = lowerBound;
	if (!twisted && divisions%2 != 0)
		y = y-1 < 0 ? divisions-1 : y-1;

	length = divisions/2 + (!twisted || divisions%2 == 0 ? 0 : 1);
	for (int i = 0; i < length; i++) {
		resizeIndexBuffer(3);
		if (twisted) {
			indices[ibIndex++] = c + (x-1 < 0 ? divisions-1 : x-1);
			indices[ibIndex++] = c + x;
			indices[ibIndex++] = b + y;
			y = y+1 < divisions ? y+1 : 0;
			x = x-1 < 0 ? divisions-1 : x-1;
		} else {
			indices[ibIndex++] = b + x;
			indices[ibIndex++] = b + (x+1 < divisions ? x+1 : 0);
			indices[ibIndex++] = c + y;
			y = y-1 < 0 ? divisions-1 : y-1;
			x = x+1 < divisions ? x+1 : 0;
		}
	}
}

void MeshGenerator::addDichotomousStems(Stem *parent, size_t crossSection)
{
	Path path = parent->getDichotomousStem(1)->getPath();
	float offset = parent->minRadius * 2.0f;
	Vec3 direction = path.getDirection(0);
	int divisions = parent->getResolution();
	size_t b = vbIndex;
	size_t c;

	addStem(parent->getDichotomousStem(0), offset);

	c = vbIndex;

	connectDichotomous(crossSection, b, c, divisions, direction);
	addStem(parent->getDichotomousStem(1), offset);
}

void MeshGenerator::addStem(Stem *stem, float offset)
{
	stem->vertexStart = vbIndex;
	stem->indexStart = ibIndex;

	{
		size_t origIndex = vbIndex;
		addCrossSection(stem, 0, offset);
		addTriangleRing(origIndex/6, vbIndex/6, stem->getResolution());
	}

	for (int i = 1; i < stem->getPath().getDivisions() - 1; i++) {
		size_t origIndex = vbIndex;
		addCrossSection(stem, i);
		addTriangleRing(origIndex/6, vbIndex/6, stem->getResolution());
	}

	if (stem->hasDichotomousStems()) {
		size_t crossSection = vbIndex;
		float o = -0.05f;
		addCrossSection(stem, stem->getPath().getDivisions() - 1, o);
		stem->vertexCount = vbIndex - stem->vertexStart;
		stem->indexCount = ibIndex - stem->indexStart;
		addDichotomousStems(stem, crossSection);
	} else {
		size_t crossSection = vbIndex;
		addCrossSection(stem, stem->getPath().getDivisions() - 1);
		capStem(stem, crossSection);
		stem->vertexCount = vbIndex - stem->vertexStart;
		stem->indexCount = ibIndex - stem->indexStart;
	}

	for (size_t i = 0; i < stem->getChildCount(); i++)
		addStem(stem->getChild(i), 0.0f);
}

bool MeshGenerator::generate(Stem *stem)
{
	resized = false;
	vbIndex = 0;
	ibIndex = 0;
	addStem(stem);
	return resized;
}

const float *MeshGenerator::getVertices()
{
	return &vertices[0];
}

size_t MeshGenerator::getVertexCount()
{
	return vbIndex;
}

size_t MeshGenerator::getVertexCapacity()
{
	return vertices.size();
}

const unsigned *MeshGenerator::getIndices()
{
	return &indices[0];
}

size_t MeshGenerator::getIndexCount()
{
	return ibIndex;
}

size_t MeshGenerator::getIndexCapacity()
{
	return indices.size();
}

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

#include "leaf.h"

using namespace pg;

Leaf::Leaf() :
	custom(false),
	material(0),
	mesh(0),
	position(-1.0f),
	scale(1.0f, 1.0f, 1.0f),
	rotation(0.0f, 0.0f, 0.0f, 1.0f)
{

}

bool Leaf::operator==(const Leaf &leaf) const
{
	return (
		this->position == leaf.position &&
		this->scale == leaf.scale &&
		this->material == leaf.material &&
		this->rotation == leaf.rotation &&
		this->mesh == leaf.mesh &&
		this->custom == leaf.custom);
}

bool Leaf::operator!=(const Leaf &leaf) const
{
	return !(*this == leaf);
}

void Leaf::setCustom(bool custom)
{
	this->custom = custom;
}

bool Leaf::isCustom() const
{
	return this->custom;
}

void Leaf::setPosition(float position)
{
	this->position = position;
}

float Leaf::getPosition() const
{
	return this->position;
}

void Leaf::setRotation(const LeafData &data, float position, const Path &path,
	int index)
{
	if (path.getSize() < 2)
		return;

	const Vec3 direction = path.getIntermediateDirection(position);
	const float length = path.getLength();
	float start = (length - data.distance);
	start *= (start >= 0.0f);

	const Vec3 y(0.0f, 1.0f, 0.0f);
	const Vec3 z(0.0f, 0.0f, 1.0f);
	const Vec3 c = direction == z ? y : direction;
	const Vec3 b = normalize(cross(c, z));
	const Vec3 a = normalize(cross(b, c));
	Quat rotation = toBasis(a, b, c);

	/* Rotate the leaf around the stem. */
	rotation = fromAxisAngle(c, data.rotation*index) * rotation;

	/* Rotate the leaf surface upward. */
	Vec3 u = lerp(a, c, data.localUp);
	rotation = rotateIntoVecQ(c, u) * rotation;
	Vec3 v = rotate(rotation, z);
	Vec3 w = lerp(a, z, data.globalUp);
	float ratio = (length - position) / (length - start);
	float mix = data.maxUp - (data.maxUp + data.minUp) * ratio;
	rotation = rotateIntoVecQ(u, lerp(v, w, mix)) * rotation;

	/* Rotate the leaf into the stem direction. */
	v = rotate(rotation, y);
	mix = data.maxForward;
	mix -= (data.maxForward + data.minForward) * ratio;
	rotation = rotateIntoVecQ(v, lerp(v, c, mix)) * rotation;

	/* Pull the leaf downward or upward. */
	v = rotate(rotation, y);
	mix = data.verticalPull;
	rotation = rotateIntoVecQ(v, lerp(v, z, mix)) * rotation;

	this->rotation = normalize(rotation);
}

void Leaf::setRotation(Quat rotation)
{
	this->rotation = rotation;
}

Quat Leaf::getRotation() const
{
	return this->rotation;
}

void Leaf::setScale(Vec3 scale)
{
	this->scale = scale;
}

Vec3 Leaf::getScale() const
{
	return this->scale;
}

void Leaf::setMaterial(unsigned material)
{
	this->material = material;
}

unsigned Leaf::getMaterial() const
{
	return this->material;
}

void Leaf::setMesh(unsigned mesh)
{
	this->mesh = mesh;
}

unsigned Leaf::getMesh() const
{
	return this->mesh;
}

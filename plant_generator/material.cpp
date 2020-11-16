/* Copyright 2018 Floris Creyf
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

#include "material.h"

using pg::Material;
using pg::Vec3;

Material::Material()
{
	this->shininess = 16.0f;
	this->ambient = Vec3(0.1f, 0.1f, 0.1f);
}

void Material::setName(const char *name)
{
	this->name = name;
}

void Material::setName(std::string name)
{
	this->name = name;
}

std::string Material::getName() const
{
	return name;
}

void Material::setTexture(const char *file, int index)
{
	this->textures[index] = file;
}

void Material::setTexture(std::string file, int index)
{
	this->textures[index] = file;
}

std::string Material::getTexture(int index) const
{
	return this->textures[index];
}

void Material::setRatio(float ratio)
{
	this->ratio = ratio;
}

float Material::getRatio() const
{
	return ratio;
}

void Material::setShininess(float shininess)
{
	this->shininess = shininess;
}

float Material::getShininess() const
{
	return this->shininess;
}

void Material::setAmbient(Vec3 ambient)
{
	this->ambient = ambient;
}

Vec3 Material::getAmbient() const
{
	return this->ambient;
}

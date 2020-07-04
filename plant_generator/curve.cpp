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

#include "curve.h"

using namespace pg;

Curve::Curve()
{

}

Curve::Curve(int type)
{
	Spline spline;
	spline.setDefault(type);
	this->spline = spline;
}

Curve::Curve(Spline spline)
{
	this->spline = spline;
}

Curve::Curve(Spline spline, std::string name)
{
	this->spline = spline;
	this->name = name;
}

void Curve::setName(std::string name)
{
	this->name = name;
}

std::string Curve::getName() const
{
	return this->name;
}

void Curve::setSpline(Spline spline)
{
	this->spline = spline;
}

Spline Curve::getSpline() const
{
	return this->spline;
}
